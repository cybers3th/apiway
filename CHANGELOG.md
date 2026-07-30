# Changelog

All notable changes to this project are documented here. The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Prometheus `/metrics` endpoint on the gateway.
- Read-only `/admin/routes` API for the dashboard.
- React + TypeScript admin dashboard.
- Kubernetes manifests and a Helm chart.
- Observability stack (Prometheus + Grafana) via Compose profile.

## [1.0.0] - 2026-07-30

### Added
- C++ gateway: JWT (HS256) validation, API-key auth, RBAC, token-bucket rate
  limiting, prefix-based routing, and reverse proxy.
- Python auth service issuing signed JWTs with role→permission expansion.
- C# orders microservice with read/write RBAC.
- Ruby catalog microservice reachable via JWT or API key.
- Docker Compose orchestration and per-service Dockerfiles.

[Unreleased]: https://github.com/Anhedonicc/apiway/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/Anhedonicc/apiway/releases/tag/v1.0.0
