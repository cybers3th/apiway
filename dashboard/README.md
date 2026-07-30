# Apiway Dashboard

A React + TypeScript admin console for the Apiway gateway. It signs in through
the gateway's public `/auth/login`, then reads the live routing table from the
gateway's `/admin/routes` API and lets you send test requests through the
gateway to observe authentication, RBAC and rate limiting.

## Development

```bash
npm install
npm run dev   # http://localhost:5173, proxies /gw -> http://localhost:8080
```

Make sure the gateway (and, for login, the auth service) are running. The
easiest way is `docker compose up` from the repository root, then run the
dashboard against it, or just use the containerized dashboard on port 3000.

## Configuration

| Env var             | Default | Purpose                                  |
|---------------------|---------|------------------------------------------|
| `VITE_GATEWAY_BASE` | `/gw`   | Prefix the dashboard calls the gateway on |

In production the bundled nginx proxies `/gw/*` to the `gateway` service.

## Pages

- **Overview** — identity, permission summary, and route counts.
- **Routes** — the live gateway routing table and its access policy.
- **Playground** — send requests through the gateway with or without your token.
