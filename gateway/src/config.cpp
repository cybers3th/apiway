#include "config.hpp"

#include <yaml-cpp/yaml.h>

#include <cstdlib>
#include <stdexcept>

namespace {

std::vector<std::string> string_list(const YAML::Node& node) {
    std::vector<std::string> out;
    if (node && node.IsSequence())
        for (const auto& e : node) out.push_back(e.as<std::string>());
    return out;
}

}  // namespace

Config load_config(const std::string& path) {
    YAML::Node root = YAML::LoadFile(path);
    Config cfg;

    if (root["server"] && root["server"]["addr"])
        cfg.server_addr = root["server"]["addr"].as<std::string>();

    if (root["jwt"]) {
        const auto& j = root["jwt"];
        if (j["secret_env"]) cfg.jwt_secret_env = j["secret_env"].as<std::string>();
        if (j["issuer"]) cfg.jwt_issuer = j["issuer"].as<std::string>();
    }
    if (const char* secret = std::getenv(cfg.jwt_secret_env.c_str()))
        cfg.jwt_secret = secret;
    if (cfg.jwt_secret.empty())
        throw std::runtime_error("jwt signing secret is empty (set " +
                                 cfg.jwt_secret_env + ")");

    if (root["rate_limit"] && root["rate_limit"]["default"]) {
        const auto& d = root["rate_limit"]["default"];
        if (d["rps"]) cfg.default_rate_limit.rps = d["rps"].as<double>();
        if (d["burst"]) cfg.default_rate_limit.burst = d["burst"].as<double>();
    }
    if (cfg.default_rate_limit.rps <= 0) cfg.default_rate_limit.rps = 10;
    if (cfg.default_rate_limit.burst <= 0) cfg.default_rate_limit.burst = 20;

    if (root["api_keys"] && root["api_keys"].IsSequence()) {
        for (const auto& k : root["api_keys"]) {
            ApiKeyCfg a;
            a.key = k["key"].as<std::string>();
            a.name = k["name"] ? k["name"].as<std::string>() : a.key;
            a.roles = string_list(k["roles"]);
            a.permissions = string_list(k["permissions"]);
            cfg.api_keys.push_back(std::move(a));
        }
    }

    if (!root["routes"] || !root["routes"].IsSequence() || root["routes"].size() == 0)
        throw std::runtime_error("no routes configured");

    for (const auto& r : root["routes"]) {
        RouteCfg rt;
        rt.path_prefix = r["path_prefix"].as<std::string>();
        rt.upstream = r["upstream"].as<std::string>();
        rt.auth = r["auth"] && r["auth"].as<bool>();
        rt.strip_prefix = r["strip_prefix"] && r["strip_prefix"].as<bool>();
        rt.allow_api_key = r["allow_api_key"] && r["allow_api_key"].as<bool>();
        rt.required_permissions = string_list(r["required_permissions"]);

        if (r["method_permissions"]) {
            for (const auto& it : r["method_permissions"])
                rt.method_permissions[it.first.as<std::string>()] = string_list(it.second);
        }
        if (r["rate_limit"]) {
            RateLimitRule rl;
            rl.rps = r["rate_limit"]["rps"] ? r["rate_limit"]["rps"].as<double>()
                                            : cfg.default_rate_limit.rps;
            rl.burst = r["rate_limit"]["burst"] ? r["rate_limit"]["burst"].as<double>()
                                                : cfg.default_rate_limit.burst;
            rt.rate_limit = rl;
        }
        cfg.routes.push_back(std::move(rt));
    }
    return cfg;
}
