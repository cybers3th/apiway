# Observability

## Metrics

The gateway exposes Prometheus metrics at `GET /metrics`:

| Metric | Type | Labels | Meaning |
|--------|------|--------|---------|
| `apiway_requests_total` | counter | `route`, `status_class` | Proxied requests |
| `apiway_denied_total` | counter | `reason` | Requests denied at the gateway |
| `apiway_handled_total` | counter | — | All requests handled |

`reason` is one of `no_route`, `unauthorized`, `forbidden`, `rate_limited`.

## Prometheus + Grafana

```bash
docker compose -f docker-compose.yml -f docker-compose.observability.yml up --build
```

- Prometheus scrapes the gateway every 10s (`monitoring/prometheus.yml`).
- Grafana auto-provisions the Prometheus datasource and a starter dashboard
  (`monitoring/grafana/`). Open http://localhost:3001.

## Useful queries

```promql
# Request rate by route
sum by (route) (rate(apiway_requests_total[1m]))

# Denials by reason
sum by (reason) (rate(apiway_denied_total[1m]))

# 4xx/5xx share
sum(rate(apiway_requests_total{status_class=~"4|5"}[5m]))
  / sum(rate(apiway_requests_total[5m]))
```

## Health

`GET /healthz` returns `{"status":"ok"}` and is used by the Compose healthcheck
and Kubernetes probes.
