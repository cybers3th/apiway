// Self-contained SHA-256, HMAC-SHA256 and base64url. Kept dependency-free so the
// gateway needs no crypto library to verify HS256 JWTs.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace crypto {

inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

class SHA256 {
public:
    SHA256() { reset(); }

    void reset() {
        state_[0] = 0x6a09e667; state_[1] = 0xbb67ae85;
        state_[2] = 0x3c6ef372; state_[3] = 0xa54ff53a;
        state_[4] = 0x510e527f; state_[5] = 0x9b05688c;
        state_[6] = 0x1f83d9ab; state_[7] = 0x5be0cd19;
        bitlen_ = 0; buflen_ = 0;
    }

    void update(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            buf_[buflen_++] = data[i];
            if (buflen_ == 64) { transform(buf_); bitlen_ += 512; buflen_ = 0; }
        }
    }

    void update(const std::string& s) {
        update(reinterpret_cast<const uint8_t*>(s.data()), s.size());
    }

    std::array<uint8_t, 32> digest() {
        std::array<uint8_t, 32> out{};
        uint64_t total = bitlen_ + static_cast<uint64_t>(buflen_) * 8;

        size_t i = buflen_;
        buf_[i++] = 0x80;
        if (i > 56) { while (i < 64) buf_[i++] = 0; transform(buf_); i = 0; }
        while (i < 56) buf_[i++] = 0;
        for (int j = 7; j >= 0; --j) buf_[i++] = static_cast<uint8_t>(total >> (j * 8));
        transform(buf_);

        for (int k = 0; k < 8; ++k) {
            out[k * 4]     = static_cast<uint8_t>(state_[k] >> 24);
            out[k * 4 + 1] = static_cast<uint8_t>(state_[k] >> 16);
            out[k * 4 + 2] = static_cast<uint8_t>(state_[k] >> 8);
            out[k * 4 + 3] = static_cast<uint8_t>(state_[k]);
        }
        reset();
        return out;
    }

private:
    void transform(const uint8_t* chunk) {
        static const uint32_t K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (chunk[i * 4] << 24) | (chunk[i * 4 + 1] << 16) |
                   (chunk[i * 4 + 2] << 8) | chunk[i * 4 + 3];
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];
        uint32_t e = state_[4], f = state_[5], g = state_[6], h = state_[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t t1 = h + S1 + ch + K[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + maj;
            h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }

        state_[0] += a; state_[1] += b; state_[2] += c; state_[3] += d;
        state_[4] += e; state_[5] += f; state_[6] += g; state_[7] += h;
    }

    uint32_t state_[8];
    uint64_t bitlen_;
    uint8_t buf_[64];
    size_t buflen_;
};

inline std::array<uint8_t, 32> sha256(const std::string& s) {
    SHA256 h; h.update(s); return h.digest();
}

inline std::string hmac_sha256(const std::string& key, const std::string& msg) {
    const size_t block = 64;
    std::string k = key;
    if (k.size() > block) {
        auto d = sha256(k);
        k.assign(reinterpret_cast<char*>(d.data()), d.size());
    }
    k.resize(block, '\0');

    std::string ipad(block, 0), opad(block, 0);
    for (size_t i = 0; i < block; ++i) {
        ipad[i] = static_cast<char>(k[i] ^ 0x36);
        opad[i] = static_cast<char>(k[i] ^ 0x5c);
    }

    SHA256 inner; inner.update(ipad); inner.update(msg);
    auto ih = inner.digest();

    SHA256 outer; outer.update(opad);
    outer.update(std::string(reinterpret_cast<char*>(ih.data()), ih.size()));
    auto oh = outer.digest();

    return std::string(reinterpret_cast<char*>(oh.data()), oh.size());
}

inline std::string base64url_encode(const std::string& in) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    int val = 0, bits = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        bits += 8;
        while (bits >= 0) { out.push_back(tbl[(val >> bits) & 0x3F]); bits -= 6; }
    }
    if (bits > -6) out.push_back(tbl[((val << 8) >> (bits + 8)) & 0x3F]);
    return out;
}

inline std::string base64url_decode(const std::string& in) {
    auto dec = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '-') return 62;
        if (c == '_') return 63;
        return -1;
    };
    std::string out;
    int val = 0, bits = -8;
    for (char c : in) {
        int d = dec(c);
        if (d < 0) continue;
        val = (val << 6) + d;
        bits += 6;
        if (bits >= 0) { out.push_back(static_cast<char>((val >> bits) & 0xFF)); bits -= 8; }
    }
    return out;
}

}  // namespace crypto
