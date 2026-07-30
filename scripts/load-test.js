// k6 load test for the Apiway gateway.
//   k6 run -e BASE=http://localhost:8080 -e TOKEN=<jwt> scripts/load-test.js
import http from "k6/http";
import { check, sleep } from "k6";

export const options = {
  stages: [
    { duration: "15s", target: 20 },
    { duration: "30s", target: 50 },
    { duration: "15s", target: 0 },
  ],
  thresholds: {
    http_req_duration: ["p(95)<250"],
  },
};

const BASE = __ENV.BASE || "http://localhost:8080";
const TOKEN = __ENV.TOKEN || "";

export default function () {
  const headers = TOKEN ? { Authorization: `Bearer ${TOKEN}` } : {};
  const res = http.get(`${BASE}/api/catalog`, { headers });
  // 429s are expected under load once the rate limiter kicks in.
  check(res, {
    "status ok or rate-limited": (r) => r.status === 200 || r.status === 429,
  });
  sleep(0.1);
}
