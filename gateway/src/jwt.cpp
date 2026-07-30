#include "jwt.hpp"

#include <yaml-cpp/yaml.h>

#include <chrono>
#include <cstdint>

#include "crypto.hpp"

namespace {

// YAML is a superset of JSON, so yaml-cpp parses a decoded JWT segment directly.
std::vector<std::string> string_list(const YAML::Node& node) {
    std::vector<std::string> out;
    if (node && node.IsSequence())
        for (const auto& e : node) out.push_back(e.as<std::string>());
    return out;
}

bool constant_time_equal(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

}  // namespace

std::optional<Principal> JwtValidator::validate(const std::string& token) const {
    auto dot1 = token.find('.');
    if (dot1 == std::string::npos) return std::nullopt;
    auto dot2 = token.find('.', dot1 + 1);
    if (dot2 == std::string::npos) return std::nullopt;

    const std::string header_b64 = token.substr(0, dot1);
    const std::string payload_b64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
    const std::string signature_b64 = token.substr(dot2 + 1);
    const std::string signing_input = token.substr(0, dot2);

    // Reject anything that is not HS256.
    try {
        YAML::Node header = YAML::Load(crypto::base64url_decode(header_b64));
        if (!header["alg"] || header["alg"].as<std::string>() != "HS256")
            return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }

    const std::string expected =
        crypto::base64url_encode(crypto::hmac_sha256(secret_, signing_input));
    if (!constant_time_equal(expected, signature_b64)) return std::nullopt;

    try {
        YAML::Node claims = YAML::Load(crypto::base64url_decode(payload_b64));

        if (claims["exp"]) {
            auto exp = claims["exp"].as<int64_t>();
            auto now = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
            if (now >= exp) return std::nullopt;
        }
        if (!issuer_.empty() && claims["iss"] &&
            claims["iss"].as<std::string>() != issuer_) {
            return std::nullopt;
        }

        Principal p;
        p.auth_type = "jwt";
        if (claims["sub"]) p.subject = claims["sub"].as<std::string>();
        if (claims["username"]) p.username = claims["username"].as<std::string>();
        p.roles = string_list(claims["roles"]);
        p.permissions = string_list(claims["permissions"]);
        return p;
    } catch (...) {
        return std::nullopt;
    }
}
