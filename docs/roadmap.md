# Roadmap

This is a direction, not a promise. Contributions toward any of these are
welcome — see [CONTRIBUTING.md](../CONTRIBUTING.md).

## Near term
- [ ] Redis-backed distributed rate limiting (shared across gateway replicas)
- [ ] RS256 / JWKS support for asymmetric token verification
- [ ] Hot-reload of `config.yaml` without a restart
- [ ] Per-route request/response size limits

## Mid term
- [ ] Admin API for managing API keys at runtime (CRUD)
- [ ] Circuit breaking and upstream health checks
- [ ] OpenTelemetry tracing across the gateway and services
- [ ] gRPC upstream support

## Longer term
- [ ] Pluggable auth backends (OIDC, mTLS)
- [ ] WebSocket proxying
- [ ] Policy-as-code (OPA) integration for complex authorization
