#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "principal.hpp"

// APIKeyStore is a concurrency-safe registry of API keys mapped to the identity
// they represent. Keys live in memory; swap the backing map for a database or
// secret manager without touching the gateway.
class ApiKeyStore {
public:
    void add(const std::string& key, const std::string& name,
             const std::vector<std::string>& roles,
             const std::vector<std::string>& permissions) {
        std::lock_guard<std::mutex> lk(mu_);
        Principal p;
        p.subject = name;
        p.username = name;
        p.auth_type = "api_key";
        p.roles = roles;
        p.permissions = permissions;
        keys_[key] = std::move(p);
    }

    std::optional<Principal> lookup(const std::string& key) const {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = keys_.find(key);
        if (it == keys_.end()) return std::nullopt;
        return it->second;
    }

private:
    mutable std::mutex mu_;
    std::unordered_map<std::string, Principal> keys_;
};
