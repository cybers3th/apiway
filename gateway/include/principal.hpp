#pragma once

#include <string>
#include <vector>

// Principal is the authenticated identity attached to a request, whether it was
// proven by a JWT or an API key.
struct Principal {
    std::string subject;
    std::string username;
    std::string auth_type;  // "jwt" or "api_key"
    std::vector<std::string> roles;
    std::vector<std::string> permissions;

    // The wildcard "*" grants every permission.
    bool has_permission(const std::string& perm) const {
        for (const auto& held : permissions)
            if (held == "*" || held == perm) return true;
        return false;
    }

    // Returns the first missing permission, or "" when all are held.
    std::string first_missing(const std::vector<std::string>& required) const {
        for (const auto& need : required)
            if (!has_permission(need)) return need;
        return "";
    }
};
