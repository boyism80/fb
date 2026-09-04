# Docker Compose — local fb stack

Single-world development stack using Docker Compose.

## Prerequisites

- Docker with Compose v2
- Git submodules initialized
- Bash (Git Bash or WSL on Windows)

## Quick start

From the repository root:

```bash
cp infra/compose/.env.example infra/compose/.env
# Edit FB_HOST if clients connect from another machine (LAN IP)

bash tools/compose-config.sh   # render gateway/login/game/game-cross configs
bash tools/compose-build.sh    # build fb/data, fb/build, app images (long first run)
docker compose -f infra/compose/docker-compose.yml --profile infra --profile app up -d
```

Or use the wrapper:

```bash
bash tools/compose-up.sh
```

## Profiles

| Profile | Services |
|---------|----------|
| `infra` | mysql, redis, rabbitmq |
| `app` | internal, write-back, log, marketplace, matchmaking, admin-tool, gateway, login, game, game-cross |

Infra only:

```bash
docker compose -f infra/compose/docker-compose.yml --profile infra up -d
```

## Ports (defaults)

| Service | Host port |
|---------|-----------|
| MySQL | 3306 |
| Redis | 6379 |
| RabbitMQ | 5672 / 15672 (mgmt) |
| Internal | 3000 |
| Gateway | 3001 |
| Login | 3002 |
| Game | 3004 |
| Game (cross) | 3100 |
| Marketplace | 3010 |
| Matchmaking | 3340 |
| Admin Tool | 3020 |

## Configuration

- **`.env`** — ports and `FB_HOST` (client-facing IP)
- **`config/*/appsettings.Development.json`** — .NET services (Docker DNS: `mysql`, `redis`, …)
- **`config/*/config.json.template`** — C++ services; rendered by `tools/compose-config.sh`

Generated files (gitignored): `config/gateway/config.json`, `config/login/config.json`, `config/game/config.json`, `config/game-cross/config.json`.

## Build notes

Compose uses the same `server/*/Dockerfile` definitions as CI. Local builds pass `BUILD_IMAGE=fb/build:local` and `DATA_IMAGE=fb/data:local` via compose build args (defaults in Dockerfiles point at GHCR).

Build order is enforced by `tools/compose-build.sh`:

1. `server/fb/data/Dockerfile`
2. `server/fb/Dockerfile`
3. Application images via `docker compose build`

## Tear down

```bash
docker compose -f infra/compose/docker-compose.yml --profile infra --profile app down
docker compose -f infra/compose/docker-compose.yml down -v   # remove volumes
```

See [wiki: Linux — Docker Compose](../../wiki/Installation-Linux-Compose.md).
