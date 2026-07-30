# Development plan

An incremental, ship-small-often plan. The aim is a steady cadence of roughly
**five meaningful commits per day**, each a self-contained, reviewable change
that builds and passes CI — not filler. Items are drawn from the
[roadmap](roadmap.md) and real gaps.

> A healthy contribution graph is a *side effect* of real work. Empty or
> cosmetic "streak" commits are easy to spot and hurt credibility with the
> exact audience that stars infrastructure projects. Everything below is real.

## Daily shape (template)

A good day usually contains one of each:

1. **Feature** — a user-visible capability or endpoint.
2. **Test** — coverage for that feature (or a gap).
3. **Docs** — reflect the change in `docs/` / README / OpenAPI.
4. **Refactor/chore** — tidy, dependency bump, or CI tweak.
5. **Fix/polish** — a bug fix or small UX/perf improvement.

Use [Conventional Commits](https://www.conventionalcommits.org/):
`feat(gateway): …`, `test(auth): …`, `docs: …`, `refactor: …`, `fix: …`.

## Sprint 1 — two weeks

### Day 1 — Distributed rate limiting
- `feat(gateway): add rate_limit.backend config (memory | redis)`
- `feat(gateway): Redis token-bucket store behind a Limiter interface`
- `feat(gateway): fall back to in-memory limiter when Redis is absent`
- `test(gateway): limiter interface behavior across backends`
- `docs(configuration): document rate-limit backends`

### Day 2 — Asymmetric tokens (RS256 / JWKS)
- `feat(gateway): verify RS256 tokens with a configured public key`
- `feat(gateway): fetch and cache JWKS from the issuer`
- `feat(auth-service): optional RS256 signing with an RSA keypair`
- `test: RS256 accept/reject and key-rotation cases`
- `docs(rbac): asymmetric verification section`

### Day 3 — Config hot-reload
- `feat(gateway): watch the config file for changes`
- `feat(gateway): atomically swap the route table on reload`
- `feat(gateway): POST /admin/reload (admin) to trigger reload`
- `test(gateway): serving continues across a reload`
- `docs: hot-reload behavior and caveats`

### Day 4 — Runtime API-key management
- `feat(gateway): POST/DELETE /admin/api-keys`
- `feat(gateway): persist API keys to the configured store`
- `feat(dashboard): API Keys management page`
- `test: API-key CRUD and auth checks`
- `docs(api): document admin API-key endpoints`

### Day 5 — Observability depth
- `feat(gateway): request-latency histogram metric`
- `feat(gateway): per-upstream error counter`
- `feat(gateway): structured JSON access logs`
- `chore(monitoring): add latency panels to the Grafana dashboard`
- `docs(observability): new metrics and log fields`

### Day 6 — Resilience
- `feat(gateway): passive per-upstream health tracking`
- `feat(gateway): open the circuit after N consecutive failures`
- `feat(gateway): half-open probe and recovery`
- `test(gateway): circuit-breaker state machine`
- `docs: resilience and failure modes`

### Day 7 — Tracing
- `feat(gateway): emit OpenTelemetry spans for proxied requests`
- `feat(gateway): propagate trace context to upstreams`
- `feat(auth-service): OTel instrumentation`
- `build: add an OTel collector to the observability stack`
- `docs(observability): tracing quickstart`

### Day 8 — Dashboard UX
- `feat(dashboard): metrics page with charts from /metrics`
- `feat(dashboard): live request log stream`
- `feat(dashboard): light/dark theme toggle`
- `test(dashboard): component tests with Vitest`
- `chore(ci): run dashboard tests in CI`

### Day 9 — Hardening
- `feat(gateway): request/response body size limits`
- `feat(gateway): configurable CORS`
- `fix(gateway): normalize and reject ambiguous paths`
- `test(gateway): limits and CORS`
- `docs(security): hardening guide`

### Day 10 — Release
- `build: multi-arch images with docker buildx`
- `chore: bump version and update CHANGELOG for v1.1.0`
- `ci: automate release notes`
- `docs: upgrade guide 1.0 -> 1.1`
- `chore(release): tag v1.1.0`

## After the sprint

Repeat the **Daily shape** template, pulling the next roadmap item. Keep each
commit green, keep PRs small, and write the changelog entry as you go.
