# Role-Based Access Control

Apiway uses a two-layer model:

1. **Roles → permissions** are defined once, in the auth service
   (`services/auth-service/app/security.py`). When a user logs in, their roles
   are expanded into a flat permission list that is baked into the JWT.
2. **Routes → required permissions** are declared in `gateway/config.yaml`. The
   gateway checks the token's permissions against the route policy — it never
   needs to know about roles.

This means the gateway enforces authorization with zero round trips: everything
it needs is in the signed token.

## Default roles

| Role | Permissions |
|------|-------------|
| `admin` | `*` (wildcard — grants everything, including `admin:read`) |
| `user` | `catalog:read`, `orders:read` |
| `orders-manager` | `orders:read`, `orders:write` |

## Permission checks

- A route lists `required_permissions`; the principal must hold **all** of them.
- `method_permissions` overrides the requirement for a specific HTTP method
  (e.g. `GET` needs `orders:read` while `POST` needs `orders:write`).
- The wildcard permission `*` satisfies any check.

## Example

```yaml
# gateway/config.yaml
- path_prefix: /api/orders
  auth: true
  required_permissions: [orders:read]
  method_permissions:
    POST: [orders:write]
    PUT: [orders:write]
    DELETE: [orders:write]
```

A `user` can `GET /api/orders` but a `POST` returns `403`. An `orders-manager`
can do both.

## Adding a permission

1. Add it to the relevant role(s) in `security.py`.
2. Reference it in the route policy in `config.yaml`.
3. Re-issue tokens (users log in again).
