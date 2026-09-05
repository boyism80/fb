# Kubernetes — fb dev overlay

Plain Kubernetes manifests (kustomize) aligned with `infra/pulumi/` resource shapes.
Single world (`dev`), minimal shard counts — same StatefulSet/Service naming as Pulumi production.

Pulumi-based deploy is optional; see `infra/pulumi/` and [wiki: Linux — Pulumi](../../wiki/Installation-Linux-Pulumi.md).

## Layout

```
infra/k8s/
  base/           namespace
  infra/          mysql, redis, rabbitmq (StatefulSets)
  apps/           gateway, login, game, game-cross (StatefulSets) + .NET + crash-watch (Deployments)
  config/         appsettings.k8s.json + C++ config templates
  overlays/dev/   kustomize entry point
```

## Dev world (minimal)

| Resource | Kind | Replicas |
|----------|------|----------|
| `mysql-unified`, `mysql-dev-global`, `mysql-dev-0`, `mysql-dev-log-0` | StatefulSet | 1 each |
| `redis-unified`, `redis-dev-global`, `redis-dev-0` | StatefulSet | 1 each |
| `rabbitmq-internal`, `rabbitmq-log` | StatefulSet | 1 each |
| `gateway`, `login-dev`, `game-dev-0`, `game-cross-200`, `game-cross-201`, `game-cross-202` | StatefulSet | 1 each |
| `internal`, `log-dev`, `write-back-dev`, `marketplace`, `matchmaking`, `admin-tool`, `crash-watch` | Deployment | 1 each |

Production (Pulumi) scales shards/replicas up using the same naming pattern.

## Prerequisites

- Kubernetes cluster + `kubectl`
- Docker images built locally (`bash tools/compose-build.sh`) or pushed to your registry
- Host paths under `/mnt/fb/` (or label a node `infra=true` and ensure paths exist)
- For local clusters (kind, minikube): load images into the cluster

```bash
# example: kind
kind load docker-image fb/game:local
# … repeat for fb/internal:local, fb/gateway:local, fb/crash-watch:local, etc.
```

## Configure client address

Edit `infra/k8s/config/fb-host.env` (copy from `fb-host.env.example`) or set `FB_HOST`:

```bash
export FB_HOST=192.168.1.100
bash tools/k8s-config.sh
```

This renders C++ `config.json` files with the IP clients use (NodePort on node IP).

## Deploy

```bash
bash tools/k8s-config.sh
kubectl apply -k infra/k8s/overlays/dev
kubectl -n fb get pods
```

## NodePorts (dev overlay)

| Service | NodePort |
|---------|----------|
| Gateway | 30000 |
| Login | 30010 |
| Game | 30030 |
| Game (cross) | 30320, 30321, 30322 |
| Internal | 30200 |
| Admin Tool | 30210 |
| Marketplace | 30220 |
| Matchmaking | 30240 |
| MySQL unified | 31000 |
| Redis unified | 31010 |
| RabbitMQ internal AMQP | 31020 |

Connect clients to `<node-ip>:30000` (gateway). Ensure `FB_HOST` in C++ configs matches the reachable node IP.

C++ servers always write daily files to `logs/YYYY-MM-DD-{server}.log` in the container working directory (`game`, `login`, `gateway`). Instance id is in each line. Those files are ephemeral unless you copy them out; crash-watch still uses container logs for Discord.

## Crash watch

`crash-watch` watches gateway, login, and game pods (home and cross share `app: game`) and posts Discord when a container exits non-zero. Leave the webhook empty to log only.

```bash
kubectl -n fb patch secret discord-webhook --type merge \
  -p '{"stringData":{"url":"https://discord.com/api/webhooks/..."}}'
```

## Custom registry

Edit `overlays/dev/kustomization.yaml` `images:` section:

```yaml
- name: fb/game
  newName: ghcr.io/myorg/fb/game
  newTag: latest
```

Set `imagePullPolicy: Always` in app manifests if needed.

## Tear down

```bash
kubectl delete -k infra/k8s/overlays/dev
```

See [wiki: Linux — Kubernetes](../../wiki/Installation-Linux-Kubernetes.md).
