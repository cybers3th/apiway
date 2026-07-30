#!/usr/bin/env bash
# Print a fresh random secret suitable for JWT_SECRET.
set -euo pipefail

if command -v openssl >/dev/null 2>&1; then
  openssl rand -hex 32
elif command -v xxd >/dev/null 2>&1; then
  head -c 32 /dev/urandom | xxd -p -c 32
else
  head -c 32 /dev/urandom | od -An -tx1 | tr -d ' \n'
  echo
fi
