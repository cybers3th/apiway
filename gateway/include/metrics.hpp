#pragma once

#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

// Metrics collects Prometheus-style counters. It is intentionally small: the
// gateway exposes the rendered text at GET /metrics for Prometheus to scrape.
class Metrics {
public:
    void observe_request(const std::string& route, int status) {
        std::lock_guard<std::mutex> lk(mu_);
        requests_[{route, status / 100}]++;
        total_++;
    }

    void observe_denied(const std::string& reason) {
        std::lock_guard<std::mutex> lk(mu_);
        denied_[reason]++;
        total_++;
    }

    std::string render() const {
        std::lock_guard<std::mutex> lk(mu_);
        std::ostringstream os;

        os << "# HELP apiway_requests_total Proxied requests by route and status class.\n";
        os << "# TYPE apiway_requests_total counter\n";
        for (const auto& [key, count] : requests_)
            os << "apiway_requests_total{route=\"" << key.first
               << "\",status_class=\"" << key.second << "xx\"} " << count << "\n";

        os << "# HELP apiway_denied_total Requests denied at the gateway by reason.\n";
        os << "# TYPE apiway_denied_total counter\n";
        for (const auto& [reason, count] : denied_)
            os << "apiway_denied_total{reason=\"" << reason << "\"} " << count << "\n";

        os << "# HELP apiway_handled_total Total requests handled by the gateway.\n";
        os << "# TYPE apiway_handled_total counter\n";
        os << "apiway_handled_total " << total_ << "\n";
        return os.str();
    }

private:
    mutable std::mutex mu_;
    std::map<std::pair<std::string, int>, long long> requests_;
    std::map<std::string, long long> denied_;
    long long total_ = 0;
};
