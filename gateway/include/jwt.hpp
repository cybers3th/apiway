#pragma once

#include <optional>
#include <string>

#include "principal.hpp"

// JwtValidator verifies HS256 tokens against a shared secret and expected
// issuer, returning the derived principal on success.
class JwtValidator {
public:
    JwtValidator(std::string secret, std::string issuer)
        : secret_(std::move(secret)), issuer_(std::move(issuer)) {}

    std::optional<Principal> validate(const std::string& token) const;

private:
    std::string secret_;
    std::string issuer_;
};
