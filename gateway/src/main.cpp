// Apiway API gateway: matches routes, authenticates callers (JWT or API key),
// enforces RBAC and rate limits, exposes metrics and a read-only admin API, then
// reverse-proxies to the upstream microservices.

#include <httplib.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "api_key_store.hpp"
#include "config.hpp"
#include "jwt.hpp"
#include "metrics.hpp"
#include "principal.hpp"
#include "rate_limiter.hpp"

namespace {

const char* kIdentityHeaders[] = {"X-Auth-Subject", "X-Auth-Username", "X-Auth-Roles",
                                  "X-Auth-Permissions", "X-Auth-Type"};

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool has_prefix(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

std::string join(const std::vector<std::string>& v) {
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) out.push_back(',');
        out += v[i];
    }
    return out;
}

std::string json_escape(const std::string& s) {
    std::string o;
    o.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char b[8];
                    std::snprintf(b, sizeof(b), "\\u%04x", c);
                    o += b;
                } else {
                    o += c;
                }
        }
    }
    return o;
}

std::string json_str(const std::string& s) { return "\"" + json_escape(s) + "\""; }

std::string json_array(const std::vector<std::string>& v) {
    std::string o = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) o += ",";
        o += json_str(v[i]);
    }
    o += "]";
    return o;
}

void write_error(httplib::Response& res, int code, const std::string& msg) {
    res.status = code;
    res.set_content("{\"error\":\"" + json_escape(msg) + "\",\"status\":" + std::to_string(code) + "}",
                    "application/json");
}

// Splits "http://host:port/..." into host and port (default 80, or 443 for https).
bool parse_upstream(const std::string& url, std::string& host, int& port) {
    std::string u = url;
    port = 80;
    if (has_prefix(u, "https://")) { u = u.substr(8); port = 443; }
    else if (has_prefix(u, "http://")) { u = u.substr(7); }

    auto slash = u.find('/');
    if (slash != std::string::npos) u = u.substr(0, slash);

    auto colon = u.find(':');
    if (colon != std::string::npos) {
        host = u.substr(0, colon);
        port = std::stoi(u.substr(colon + 1));
    } else {
        host = u;
    }
    return !host.empty();
}

void parse_listen(const std::string& addr, std::string& host, int& port) {
    auto pos = addr.rfind(':');
    if (pos == std::string::npos) { host = "0.0.0.0"; port = 8080; return; }
    host = (pos == 0) ? "0.0.0.0" : addr.substr(0, pos);
    if (host.empty()) host = "0.0.0.0";
    port = std::stoi(addr.substr(pos + 1));
}

}  // namespace

class Gateway {
public:
    explicit Gateway(Config cfg)
        : cfg_(std::move(cfg)),
          validator_(cfg_.jwt_secret, cfg_.jwt_issuer) {
        for (const auto& k : cfg_.api_keys)
            keys_.add(k.key, k.name, k.roles, k.permissions);

        // Match the most specific (longest) prefix first.
        std::sort(cfg_.routes.begin(), cfg_.routes.end(),
                  [](const RouteCfg& a, const RouteCfg& b) {
                      return a.path_prefix.size() > b.path_prefix.size();
                  });

        for (const auto& r : cfg_.routes) {
            RateLimitRule rl = r.rate_limit ? *r.rate_limit : cfg_.default_rate_limit;
            limiters_.push_back(std::make_unique<RateLimiter>(rl.rps, rl.burst));
        }
    }

    void handle(const httplib::Request& req, httplib::Response& res) {
        if (req.path == "/healthz") {
            res.status = 200;
            res.set_content("{\"status\":\"ok\"}", "application/json");
            return;
        }
        if (req.path == "/metrics") {
            res.status = 200;
            res.set_content(metrics_.render(), "text/plain; version=0.0.4");
            return;
        }
        if (has_prefix(req.path, "/admin")) {
            handle_admin(req, res);
            return;
        }

        int idx = match(req.path);
        if (idx < 0) {
            metrics_.observe_denied("no_route");
            write_error(res, 404, "no route matches path");
            return;
        }
        const RouteCfg& rt = cfg_.routes[idx];

        std::optional<Principal> principal;
        if (rt.auth) {
            int code = 401;
            std::string msg;
            principal = authenticate(req, rt, code, msg);
            if (!principal) {
                metrics_.observe_denied("unauthorized");
                res.set_header("WWW-Authenticate", "Bearer");
                write_error(res, code, msg);
                return;
            }
            const std::vector<std::string>* required = &rt.required_permissions;
            auto it = rt.method_permissions.find(req.method);
            if (it != rt.method_permissions.end()) required = &it->second;

            std::string missing = principal->first_missing(*required);
            if (!missing.empty()) {
                metrics_.observe_denied("forbidden");
                write_error(res, 403, "missing permission: " + missing);
                return;
            }
        }

        std::string client_key = "ip:" + req.remote_addr;
        if (principal && !principal->subject.empty())
            client_key = principal->auth_type + ":" + principal->subject;
        if (!limiters_[idx]->allow(client_key)) {
            metrics_.observe_denied("rate_limited");
            res.set_header("Retry-After", "1");
            write_error(res, 429, "rate limit exceeded");
            return;
        }

        proxy(req, res, rt, principal);
        metrics_.observe_request(rt.path_prefix, res.status);
    }

private:
    int match(const std::string& path) const {
        for (size_t i = 0; i < cfg_.routes.size(); ++i)
            if (has_prefix(path, cfg_.routes[i].path_prefix)) return static_cast<int>(i);
        return -1;
    }

    std::optional<Principal> authenticate(const httplib::Request& req, const RouteCfg& rt,
                                          int& code, std::string& msg) {
        if (rt.allow_api_key) {
            std::string key = req.get_header_value("X-API-Key");
            if (!key.empty()) {
                auto p = keys_.lookup(key);
                if (p) return p;
                code = 401; msg = "invalid api key";
                return std::nullopt;
            }
        }
        return bearer_principal(req, code, msg);
    }

    std::optional<Principal> bearer_principal(const httplib::Request& req, int& code,
                                              std::string& msg) {
        std::string auth = req.get_header_value("Authorization");
        if (auth.size() > 7 && to_lower(auth.substr(0, 7)) == "bearer ") {
            std::string token = auth.substr(7);
            while (!token.empty() && token.front() == ' ') token.erase(token.begin());
            auto p = validator_.validate(token);
            if (p) return p;
            code = 401; msg = "invalid token";
            return std::nullopt;
        }
        code = 401; msg = "missing credentials";
        return std::nullopt;
    }

    // Read-only admin API used by the dashboard. Requires an admin JWT
    // (permission "admin:read" or the "*" wildcard).
    void handle_admin(const httplib::Request& req, httplib::Response& res) {
        int code = 401;
        std::string msg;
        auto principal = bearer_principal(req, code, msg);
        if (!principal) {
            metrics_.observe_denied("unauthorized");
            res.set_header("WWW-Authenticate", "Bearer");
            write_error(res, code, msg);
            return;
        }
        if (!principal->has_permission("admin:read") && !principal->has_permission("*")) {
            metrics_.observe_denied("forbidden");
            write_error(res, 403, "admin permission required");
            return;
        }
        if (req.method == "GET" && req.path == "/admin/routes") {
            res.status = 200;
            res.set_content(routes_json(), "application/json");
            return;
        }
        write_error(res, 404, "unknown admin endpoint");
    }

    std::string routes_json() const {
        std::string o = "{\"routes\":[";
        for (size_t i = 0; i < cfg_.routes.size(); ++i) {
            const auto& r = cfg_.routes[i];
            if (i) o += ",";
            o += "{";
            o += "\"path_prefix\":" + json_str(r.path_prefix) + ",";
            o += "\"upstream\":" + json_str(r.upstream) + ",";
            o += "\"auth\":" + std::string(r.auth ? "true" : "false") + ",";
            o += "\"allow_api_key\":" + std::string(r.allow_api_key ? "true" : "false") + ",";
            o += "\"strip_prefix\":" + std::string(r.strip_prefix ? "true" : "false") + ",";
            o += "\"required_permissions\":" + json_array(r.required_permissions) + ",";
            o += "\"method_permissions\":{";
            bool first = true;
            for (const auto& [method, perms] : r.method_permissions) {
                if (!first) o += ",";
                first = false;
                o += json_str(method) + ":" + json_array(perms);
            }
            o += "}}";
        }
        o += "]}";
        return o;
    }

    void proxy(const httplib::Request& req, httplib::Response& res, const RouteCfg& rt,
               const std::optional<Principal>& principal) {
        std::string host;
        int port;
        if (!parse_upstream(rt.upstream, host, port)) {
            write_error(res, 502, "bad upstream");
            return;
        }

        // target = path + query, with optional prefix stripping on the path only.
        std::string target = req.target;
        if (rt.strip_prefix && !rt.path_prefix.empty()) {
            auto q = target.find('?');
            std::string path = (q == std::string::npos) ? target : target.substr(0, q);
            std::string query = (q == std::string::npos) ? "" : target.substr(q);
            if (has_prefix(path, rt.path_prefix)) path = path.substr(rt.path_prefix.size());
            if (path.empty() || path.front() != '/') path = "/" + path;
            target = path + query;
        }

        httplib::Request up;
        up.method = req.method;
        up.path = target;
        up.body = req.body;

        // Forward client headers, dropping hop-by-hop and any client-supplied
        // identity headers (prevents spoofing), then stamp the resolved identity.
        for (const auto& h : req.headers) {
            std::string lk = to_lower(h.first);
            if (lk == "host" || lk == "content-length" || lk == "connection" ||
                lk == "accept-encoding")
                continue;
            bool identity = false;
            for (const char* ih : kIdentityHeaders)
                if (lk == to_lower(ih)) { identity = true; break; }
            if (identity) continue;
            up.headers.emplace(h.first, h.second);
        }
        if (principal) {
            up.headers.emplace("X-Auth-Subject", principal->subject);
            up.headers.emplace("X-Auth-Username", principal->username);
            up.headers.emplace("X-Auth-Roles", join(principal->roles));
            up.headers.emplace("X-Auth-Permissions", join(principal->permissions));
            up.headers.emplace("X-Auth-Type", principal->auth_type);
        }

        httplib::Client cli(host, port);
        cli.set_connection_timeout(5, 0);
        cli.set_read_timeout(30, 0);

        if (auto result = cli.send(up)) {
            res.status = result->status;
            std::string ct = result->get_header_value("Content-Type");
            if (ct.empty()) ct = "application/octet-stream";
            res.set_content(result->body, ct);
        } else {
            std::cerr << "upstream error " << req.method << " " << req.path << "\n";
            write_error(res, 502, "upstream unavailable");
        }
    }

    Config cfg_;
    JwtValidator validator_;
    ApiKeyStore keys_;
    std::vector<std::unique_ptr<RateLimiter>> limiters_;
    Metrics metrics_;
};

int main() {
    const char* cfg_path = std::getenv("GATEWAY_CONFIG");
    std::string path = cfg_path ? cfg_path : "config.yaml";

    Config cfg;
    try {
        cfg = load_config(path);
    } catch (const std::exception& e) {
        std::cerr << "config: " << e.what() << "\n";
        return 1;
    }

    std::string listen_addr = cfg.server_addr;
    Gateway gw(std::move(cfg));

    httplib::Server svr;
    svr.set_pre_routing_handler(
        [&gw](const httplib::Request& req, httplib::Response& res) {
            gw.handle(req, res);
            return httplib::Server::HandlerResponse::Handled;
        });

    std::string host;
    int port;
    parse_listen(listen_addr, host, port);
    std::cout << "apiway gateway listening on " << host << ":" << port << "\n";
    if (!svr.listen(host, port)) {
        std::cerr << "failed to bind " << host << ":" << port << "\n";
        return 1;
    }
    return 0;
}
