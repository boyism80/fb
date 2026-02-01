# Runner vs Pulumi vs Server Schema – All Servers

## Server coverage

| Server        | Pulumi (infra/pulumi) | Runner (tools/runner) | config.dev.json (reference) |
|--------------|----------------------|------------------------|-----------------------------|
| **login**    | login.js             | config_login_{i}.json  | server/login/config/        |
| **game**     | game.js              | config_game_{id}.json  | server/game/config/         |
| **gateway**  | gateway.js           | config_gateway.json    | server/gateway/config/      |
| **internal** | internal.js          | appsettings.internal.json | (C# appsettings)         |
| **write-back** | write-back.js      | appsettings.write-back.json | (C# appsettings)       |
| **log**      | log.js               | **not generated / not run** | server/log/             |
| **admin-tool** | admin-tool.js      | **not generated / not run** | -                      |
| **marketplace** | marketplace.js    | **not generated / not run** | -                    |
| **bot**      | bot.js (commented)   | **not generated / not run** | -                      |

---

## Pulumi vs config.dev.json – alignment check

**Result: Not perfectly aligned.** Same structure (keys/nesting) for C# services; C++ configs differ in a few fields and values.

### C++ servers (login, game, gateway)

| Item | Pulumi | config.dev | Match? |
|------|--------|------------|--------|
| **thread** | `logic`, `io` only | `logic`, `io` only (removed unused `background`) | Yes |
| **login init.mp** | `base: 30`, `range: 10` | `base: 50`, `range: 10` | No – value difference. |
| **login pw_size.max** | `16` | `14` | No – value difference. |
| **game exp_multiplier** | `1.0` | `100.0` | No – value difference. |
| **game drop_rate_multiplier** | `1.0` | `100.0` | No – value difference. |
| **All other keys** | Same keys and nesting (id, name, world, amqp.internal/log, log { ip, port, level }, internal, marketplace, etc.) | Same | Yes |

So: **structure** matches (same sections and key names); **values** differ where noted (init.mp, pw_size.max, multipliers).

### C# services (internal, write-back, log)

| Item | Pulumi | config.dev (appsettings) | Match? |
|------|--------|---------------------------|--------|
| **internal** | Logging, ConnectionStrings.MySql (unified + worlds), Redis (unified + worlds), RabbitMQ (Internal/Log), Log (Enabled, ServerId, ServerName) | internal appsettings.Development.json: same structure | Yes |
| **write-back** | World, Logging, ConnectionStrings.MySql (worlds), Redis (worlds), RabbitMQ (Internal/Log), Log | write-back appsettings.json: same structure | Yes |
| **log** | World, Logging, ConnectionStrings.MySql (worlds), RabbitMQ (Internal/Log) | log appsettings.json: same | Yes |

---

## 1. Login (C++ config.json)

| Field | Pulumi (login.js) | Runner | config.dev.json (login) |
|-------|-------------------|--------|--------------------------|
| **id** | 0 | i | 0 |
| **name** | login-{worldName} | login-{i} | login-0 |
| **world** | worldConf.id | **missing** | 1 |
| **ip** | conf.host | ExternalIP | 127.0.0.1 |
| **port** | worldConf.login.port | setting.Port | 3002 |
| **thread** | logic: 32, io: 12 | logic: 12, io: 12 | logic, io |
| **amqp** | **internal** + **log** (ip, port, uid, pwd each) | **missing** | internal + log |
| **internal** | ip: "internal", port | ip: 127.0.0.1, port | ip, port |
| **log** | **object** { ip, port, level } | **array** ["debug","info","warn","fatal"] only | object { ip, port, level } |
| **transfer delay** | 0 | 0 | 0 |
| **allow_foreign_name** | false | AllowForeignName | false |
| **agreement** | long string | Agreement | short string |
| **admin_mode** | false | AdminMode | true |
| **init** | map, position, hp, mp | same | same |
| **name_size** | min: 2, max: 256 | MinIdLength, MaxIdLength | min, max |
| **pw_size** | min: 4, max: 16 | MinPwLength, MaxPwLength | min, max |

**Runner gaps:** no `world`, no `amqp`, `log` is array instead of object, thread values differ.

---

## 2. Game (C++ config.json)

| Field | Pulumi (game.js) | Runner | config.dev.json (game) |
|-------|------------------|--------|-------------------------|
| **id** | parseInt(i) | setting.ID | 0 |
| **name** | game-{worldName}-{i} | game-{setting.ID} | game-0 |
| **world** | worldConf.id | **missing** | 1 |
| **delay** | 5 | 5 | 1 |
| **ip** | conf.host | ExternalIP | 127.0.0.1 |
| **port** | container.port | setting.Port | 3004 |
| **thread** | logic: 32, io: 12 | logic: 12, io: 12 | logic, io |
| **save** | 600 | SaveInterval | 600 |
| **internal** | ip: "internal", port | ip: 127.0.0.1, port | ip, port |
| **marketplace** | ip, port | **missing** | ip, port |
| **login** | ip, port | ip, port | ip, port |
| **amqp** | **internal** + **log** (each ip, port, uid, pwd) | **single** block (ip, port, uid, pwd) | internal + log |
| **log** | **object** { ip, port, level } | **array** ["debug","info","warn","fatal"] only | object { ip, port, level } |
| **exp_multiplier** | 1.0 | **missing** | 100.0 |
| **drop_rate_multiplier** | 1.0 | **missing** | 100.0 |

**Runner gaps:** no `world`, no `marketplace`, `amqp` single block instead of internal/log, `log` array instead of object, no exp/drop_rate_multiplier.

---

## 3. Gateway (C++ config.json)

| Field | Pulumi (gateway.js) | Runner | config.dev.json (gateway) |
|-------|---------------------|--------|----------------------------|
| **id** | 0 | 0 | 0 |
| **ip** | conf.host | ExternalIP | 127.0.0.1 |
| **name** | gateway | gateway | gateway |
| **port** | conf.gateway.port | Gateway.Port | 3001 |
| **thread** | logic: 12, io: 12 | logic: 12, io: 12 | logic, io |
| **log** | **object** { ip, port, level } | **array** ["debug","info","warn","fatal"] only | object { ip, port, level } |
| **entrypoints** | from worlds | from Login list | name, desc, ip, port |
| **internal** | ip: "internal", port | **missing** | ip, port |
| **amqp** | **internal** + **log** (each ip, port, uid, pwd) | **missing** | internal + log |

**Runner gaps:** no `internal`, no `amqp`, `log` array instead of object.

---

## 4. Internal (C# appsettings)

| Field | Pulumi (internal.js) | Runner |
|-------|----------------------|--------|
| **Logging** | LogLevel Default, Microsoft.AspNetCore | same |
| **ConnectionStrings.MySql** | **unified** + **worlds[worldId].global** + **worlds[worldId].data[]** | **flat** MySql["0"], ["1"], ... |
| **Redis** | **unified** + **worlds[worldId].global** + **worlds[worldId].data[]** | **flat** Redis["0"], ["1"], ... |
| **RabbitMQ** | **Internal** + **Log** (Host, Port, Uid, Pwd each) | **single** block (Host, Port, Uid, Pwd) |
| **Log** | { Enabled, ServerId, ServerName } | **missing** |

**Runner gaps:** no worlds hierarchy, single RabbitMQ block, no Log section.

---

## 5. Write-back (C# appsettings)

| Field | Pulumi (write-back.js) | Runner |
|-------|-------------------------|--------|
| **World** | worldConf.id | **missing** |
| **Logging** | LogLevel | same |
| **ConnectionStrings.MySql** | **worlds[worldId].global** + **worlds[worldId].data[]** | **flat** MySql["0"], ["1"], ... |
| **Redis** | **worlds[worldId].global** + **worlds[worldId].data[]** | **flat** Redis["0"], ["1"], ... |
| **RabbitMQ** | **Internal** + **Log** | **single** block |
| **Log** | { Enabled, ServerId, ServerName } | **missing** |

**Runner gaps:** no World, no worlds hierarchy, single RabbitMQ, no Log section.

---

## 6. Log (C# appsettings) – Pulumi only

- **Pulumi (log.js):** World, Logging, ConnectionStrings.MySql worlds[worldId].global/data (mysql.log), RabbitMQ Internal/Log.
- **Runner:** Does not generate or run log server.

---

## 7. Admin-tool (C# appsettings) – Pulumi only

- **Pulumi (admin-tool.js):** ConnectionStrings.MySql unified + worlds, Redis unified + worlds, RabbitMQ Internal/Log, Security, WorldServers, etc.
- **Runner:** Does not generate or run admin-tool.

---

## 8. Marketplace (C# appsettings) – Pulumi only

- **Pulumi (marketplace.js):** ConnectionStrings.MySql unified + worlds, Redis unified + worlds, RabbitMQ Internal/Log.
- **Runner:** Does not generate or run marketplace.

---

## Summary

- **Runner** targets local single-world: flat MySQL/Redis lists, one RabbitMQ, no log/admin-tool/marketplace. C++ configs use `log` as array and omit `amqp` (login/gateway), `internal` (gateway), `marketplace`/`world` (game).
- **Pulumi** targets K8s multi-world: MySql/Redis worlds + unified, RabbitMQ Internal/Log, log as object, and deploys log, admin-tool, marketplace.
- **config.dev.json** (game, gateway, login) is the current server schema: `log` object, `amqp.internal`/`amqp.log`, `thread` (logic, io), `world`, `marketplace` (game), `internal` (gateway). Runner does not match this schema; Pulumi C++ configs align with it.

To make Runner align with Pulumi and config.dev.json, Runner would need to:
1. **Login:** add world, amqp.internal/log, log as object { ip, port, level }.
2. **Game:** add world, marketplace, amqp.internal/log, log as object, exp_multiplier, drop_rate_multiplier.
3. **Gateway:** add internal, amqp.internal/log, log as object.
4. **Internal/Write-back:** add Log section, RabbitMQ Internal/Log; optionally support worlds structure for local “single world” (e.g. worlds["1"]).
5. Optionally: add log/admin-tool/marketplace config generation and process startup if local full stack is desired.
