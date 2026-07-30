#!/usr/bin/env bash
# End-to-end smoke test against a running stack (docker compose up).
# Usage: BASE=http://localhost:8080 ./scripts/smoke-test.sh
set -euo pipefail

BASE="${BASE:-http://localhost:8080}"
pass=0
fail=0

# check <description> <expected-status> <actual-status>
check() {
  if [[ "$2" == "$3" ]]; then
    echo "  ok   $1 ($3)"
    pass=$((pass + 1))
  else
    echo "  FAIL $1 (expected $2, got $3)"
    fail=$((fail + 1))
  fi
}

status() { curl -s -o /dev/null -w '%{http_code}' "$@"; }

# Extract a JSON string field without requiring jq.
json_field() { sed -n "s/.*\"$1\":\"\([^\"]*\)\".*/\1/p"; }

echo "==> Health"
check "gateway healthz" 200 "$(status "$BASE/healthz")"

echo "==> Register + login"
REG=$(curl -s "$BASE/auth/register" -H 'content-type: application/json' \
  -d '{"username":"smoke-user","password":"supersecret"}' || true)
# If the user already exists, log in instead.
TOKEN=$(printf '%s' "$REG" | json_field access_token)
if [[ -z "$TOKEN" ]]; then
  LOGIN=$(curl -s "$BASE/auth/login" -H 'content-type: application/json' \
    -d '{"username":"smoke-user","password":"supersecret"}')
  TOKEN=$(printf '%s' "$LOGIN" | json_field access_token)
fi
if [[ -n "$TOKEN" ]]; then echo "  ok   obtained token"; pass=$((pass + 1));
else echo "  FAIL could not obtain token"; fail=$((fail + 1)); fi

echo "==> AuthN / AuthZ"
check "no token -> 401"          401 "$(status "$BASE/api/orders")"
check "user reads orders -> 200" 200 "$(status -H "Authorization: Bearer $TOKEN" "$BASE/api/orders")"
check "user writes orders -> 403" 403 \
  "$(status -X POST -H "Authorization: Bearer $TOKEN" -H 'content-type: application/json' \
     -d '{"item":"x","quantity":1}' "$BASE/api/orders")"
check "catalog via api key -> 200" 200 \
  "$(status -H 'X-API-Key: svc_catalog_reader_9f2c1a' "$BASE/api/catalog")"
check "bad api key -> 401" 401 \
  "$(status -H 'X-API-Key: nope' "$BASE/api/catalog")"

echo
echo "Passed: $pass  Failed: $fail"
[[ "$fail" -eq 0 ]]
