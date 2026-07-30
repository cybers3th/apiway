// Unit tests for the gateway's crypto and JWT logic. No framework: assertions
// plus a nonzero exit on failure, wired into ctest.
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>

#include "crypto.hpp"
#include "jwt.hpp"

namespace {

std::string hex(const unsigned char* p, size_t n) {
    static const char* d = "0123456789abcdef";
    std::string o;
    for (size_t i = 0; i < n; ++i) {
        o += d[p[i] >> 4];
        o += d[p[i] & 0xf];
    }
    return o;
}

long now_seconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string make_token(const std::string& secret, long exp, const std::string& iss) {
    std::string header = crypto::base64url_encode(R"({"alg":"HS256","typ":"JWT"})");
    std::string payload_json =
        std::string("{\"sub\":\"u1\",\"username\":\"alice\",") +
        "\"roles\":[\"user\"],\"permissions\":[\"orders:read\",\"catalog:read\"]," +
        "\"iss\":\"" + iss + "\",\"exp\":" + std::to_string(exp) + "}";
    std::string payload = crypto::base64url_encode(payload_json);
    std::string signing = header + "." + payload;
    std::string sig = crypto::base64url_encode(crypto::hmac_sha256(secret, signing));
    return signing + "." + sig;
}

void test_base64url_roundtrip() {
    const std::string in = "Hello, Apiway! ~ base64url +/ test";
    assert(crypto::base64url_decode(crypto::base64url_encode(in)) == in);
}

void test_sha256_known_vector() {
    auto d = crypto::sha256("abc");
    assert(hex(d.data(), d.size()) ==
           "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

void test_hmac_known_vector() {
    // RFC 4231, test case 2.
    auto mac = crypto::hmac_sha256("Jefe", "what do ya want for nothing?");
    assert(hex(reinterpret_cast<const unsigned char*>(mac.data()), mac.size()) ==
           "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
}

void test_jwt_valid() {
    const std::string secret = "test-secret";
    std::string token = make_token(secret, now_seconds() + 3600, "auth-service");
    JwtValidator v(secret, "auth-service");
    auto p = v.validate(token);
    assert(p.has_value());
    assert(p->username == "alice");
    assert(p->has_permission("orders:read"));
    assert(!p->has_permission("orders:write"));
    assert(p->first_missing({"orders:read", "catalog:read"}).empty());
    assert(p->first_missing({"orders:write"}) == "orders:write");
}

void test_jwt_rejections() {
    const std::string secret = "test-secret";
    std::string token = make_token(secret, now_seconds() + 3600, "auth-service");

    // Tampered signature.
    assert(!JwtValidator(secret, "auth-service").validate(token + "x").has_value());
    // Wrong secret.
    assert(!JwtValidator("other-secret", "auth-service").validate(token).has_value());
    // Wrong issuer.
    assert(!JwtValidator(secret, "someone-else").validate(token).has_value());
    // Expired.
    std::string expired = make_token(secret, now_seconds() - 10, "auth-service");
    assert(!JwtValidator(secret, "auth-service").validate(expired).has_value());
    // Malformed.
    assert(!JwtValidator(secret, "auth-service").validate("not-a-jwt").has_value());
}

}  // namespace

int main() {
    test_base64url_roundtrip();
    test_sha256_known_vector();
    test_hmac_known_vector();
    test_jwt_valid();
    test_jwt_rejections();
    std::printf("all gateway tests passed\n");
    return 0;
}
