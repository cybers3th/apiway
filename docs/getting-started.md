# Getting started

## With Docker (recommended)

```bash
cp .env.example .env          # set a strong JWT_SECRET (see scripts/gen-secret.sh)
docker compose up --build
```

- Gateway: http://localhost:8080
- Dashboard: http://localhost:3000
- Metrics: http://localhost:8080/metrics

Add the observability stack:

```bash
docker compose -f docker-compose.yml -f docker-compose.observability.yml up --build
# Prometheus http://localhost:9090 · Grafana http://localhost:3001
```

## Running components individually

### Gateway (C++)

Requires CMake ≥ 3.16 and a C++17 compiler. Dependencies (`cpp-httplib`,
`yaml-cpp`) are fetched automatically by CMake.

```bash
cmake -S gateway -B gateway/build -DCMAKE_BUILD_TYPE=Release
cmake --build gateway/build -j
JWT_SECRET=dev-secret ./gateway/build/gateway
```

### auth-service (Python)

```bash
cd services/auth-service
pip install -r requirements.txt
JWT_SECRET=dev-secret uvicorn app.main:app --port 9000
```

### orders-service (C#)

```bash
cd services/orders-service && dotnet run
```

### catalog-service (Ruby)

```bash
cd services/catalog-service && bundle install && ruby app.rb
```

When running outside Compose, point the `upstream` URLs in
`gateway/config.yaml` at `http://localhost:<port>`.

## First requests

```bash
# Register (public)
curl -s localhost:8080/auth/register -H 'content-type: application/json' \
  -d '{"username":"alice","password":"supersecret"}'

TOKEN=... # access_token from the response

curl -s localhost:8080/api/orders -H "Authorization: Bearer $TOKEN"
```

See the [README](../README.md#try-it) for the full walkthrough.
