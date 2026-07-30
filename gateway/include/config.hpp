#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

struct RateLimitRule {
    double rps = 0;
    double burst = 0;
};

struct ApiKeyCfg {
    std::string key;
    std::string name;
    std::vector<std::string> roles;
    std::vector<std::string> permissions;
};

struct RouteCfg {
    std::string path_prefix;
    std::string upstream;
    bool auth = false;
    bool strip_prefix = false;
    bool allow_api_key = false;
    std::vector<std::string> required_permissions;
    std::map<std::string, std::vector<std::string>> method_permissions;
    std::optional<RateLimitRule> rate_limit;
};

struct Config {
    std::string server_addr = ":8080";
    std::string jwt_secret_env = "JWT_SECRET";
    std::string jwt_issuer;
    std::string jwt_secret;  // resolved from the environment
    RateLimitRule default_rate_limit{20, 40};
    std::vector<ApiKeyCfg> api_keys;
    std::vector<RouteCfg> routes;
};

// Loads, parses and validates the YAML configuration file, resolving the JWT
// signing secret from the environment. Throws std::runtime_error on failure.
Config load_config(const std::string& path);
