#pragma once

#include <algorithm>
#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

// RateLimiter maintains an independent token bucket per client key (per
// principal or per source IP).
class RateLimiter {
public:
    RateLimiter(double rps, double burst) : rps_(rps), burst_(burst) {}

    bool allow(const std::string& key) {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lk(mu_);

        auto& b = buckets_[key];
        if (!b.seen) {
            b.tokens = burst_;
            b.last = now;
            b.seen = true;
        }
        double elapsed = std::chrono::duration<double>(now - b.last).count();
        b.last = now;
        b.tokens = std::min(burst_, b.tokens + elapsed * rps_);

        if (b.tokens >= 1.0) {
            b.tokens -= 1.0;
            return true;
        }
        return false;
    }

private:
    struct Bucket {
        double tokens = 0;
        std::chrono::steady_clock::time_point last{};
        bool seen = false;
    };

    double rps_, burst_;
    std::mutex mu_;
    std::unordered_map<std::string, Bucket> buckets_;
};
