# Configuration

The gateway reads a single YAML file (default `config.yaml`, overridable with
the `GATEWAY_CONFIG` environment variable).

## Top-level keys

```yaml
server:
  addr: ":8080"          # listen address

jwt:
  secret_env: JWT_SECRET # env var holding the HS256 signing secret
  issuer: auth-service   # expected `iss` claim (optional but recommended)

rate_limit:
  default:
    rps: 20              # tokens per second per client
    burst: 40            # bucket capacity

api_keys:                # statically provisioned machine credentials
  - key: svc_reader_9f2c1a
    name: reporting-bot
    roles: [service]
    permissions: [catalog:read]

routes:
  - path_prefix: /api/orders
    upstream: http://orders-service:9100
    auth: true
    strip_prefix: false
    allow_api_key: false
    required_permissions: [orders:read]
    method_permissions:
      POST: [orders:write]
    rate_limit:          # optional per-route override
      rps: 10
      burst: 20
```

## Route fields

| Field | Type | Meaning |
|-------|------|---------|
| `path_prefix` | string | Longest matching prefix wins |
| `upstream` | string | `http://host:port` to proxy to |
| `auth` | bool | Require a credential |
| `strip_prefix` | bool | Remove `path_prefix` before forwarding |
| `allow_api_key` | bool | Accept `X-API-Key` in addition to JWTs |
| `required_permissions` | list | All must be present on the principal |
| `method_permissions` | map | Per-HTTP-method override of the above |
| `rate_limit` | object | Per-route bucket; falls back to the default |

## Secrets

`JWT_SECRET` must match the secret the auth service signs with. Generate one
with `scripts/gen-secret.sh` and keep it in `.env` (gitignored) or a secret
manager. The gateway refuses to start if the secret is empty.
