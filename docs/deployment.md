# Deployment

## Kubernetes (raw manifests)

```bash
kubectl apply -f deploy/k8s/namespace.yaml
kubectl -n apiway create secret generic apiway-secrets \
  --from-literal=JWT_SECRET="$(openssl rand -hex 32)"
kubectl apply -f deploy/k8s/
```

Manifests included:

- `namespace.yaml`
- `gateway.yaml` — Deployment + Service + ConfigMap (the routing table)
- `auth-service.yaml`, `orders-service.yaml`, `catalog-service.yaml`
- `ingress.yaml` — exposes the gateway
- `secret.example.yaml` — template (do not commit real secrets)

All Deployments set readiness/liveness probes against `/healthz` and run as a
non-root user.

## Helm

```bash
helm install apiway deploy/helm/apiway \
  --namespace apiway --create-namespace \
  --set jwt.secret="$(openssl rand -hex 32)"
```

Tune `deploy/helm/apiway/values.yaml` for image tags, replica counts and
resource requests.

## Images

Tagging a release (`vX.Y.Z`) triggers `.github/workflows/docker-publish.yml`,
which builds and pushes every component to GHCR:

```
ghcr.io/<owner>/apiway/gateway:<tag>
ghcr.io/<owner>/apiway/auth-service:<tag>
...
```

## Production checklist

- [ ] Strong, rotated `JWT_SECRET` from a secret manager
- [ ] TLS terminated at the ingress/load balancer
- [ ] Resource requests/limits set per service
- [ ] Prometheus scraping `/metrics`; alerts on `apiway_denied_total`
- [ ] API keys provisioned out of band, not in the config file
