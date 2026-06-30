# Database Migration Design

## Goal

Implement SQL-based database migrations that run during C# server startup when enabled by `appsettings`.

EF Core migrations are intentionally not used. The current C# servers use `MySqlConnector`, Dapper, and explicit SQL. The migration system should follow the same model: versioned SQL files, explicit target database scopes, and a small runner that applies pending SQL to the configured MySQL connections.

`infra/db/latest.sql` is the source of truth for the current baseline schema. It should be split into scope-specific baseline migrations and used as the reference when creating future incremental migrations.

## Non-Goals

- Do not introduce EF Core or entity-based schema generation.
- Do not execute `infra/db/latest.sql` directly at server startup.
- Do not run destructive dump commands such as `DROP DATABASE` or `DROP TABLE` through runtime migration.
- Do not make `write-back` or `admin-tool` own schema changes.

## Startup Control

Each server reads the same configuration key:

```json
{
  "Database": {
    "AutoMigration": false
  }
}
```

`false` is the default. When the key is missing, migrations are skipped.

Recommended environment policy:

- Local development: `true`
- Staging/develop cluster: `true`
- Production: `false` by default, unless the deployment process explicitly relies on startup migration and all pending migrations are additive

The runner must execute after `builder.Build()` and before any background services or request handling depend on the schema. For web servers this means before data loading and before `app.Run()`. For worker servers this means before `host.RunAsync()`.

## Database Topology

The current Pulumi and `appsettings` shape exposes these MySQL targets:

| Scope | Configuration Path | Physical Form | World Dependent | Owner |
|---|---|---|---|---|
| `unified` | `ConnectionStrings:MySql:unified` | one MySQL instance | no | `marketplace`, shared services |
| `unified-data` | planned, for example `ConnectionStrings:MySql:unifiedData[]` | N shard MySQL instances | no | planned shared non-world data owner |
| `world-global` | `ConnectionStrings:MySql:worlds:{world}:global` | one MySQL instance per world | yes | `internal` |
| `world-data` | `ConnectionStrings:MySql:worlds:{world}:data[]` | N shard MySQL instances per world | yes | `internal`, `write-back` writes pending SQL |
| `world-log-data` | log server maps `ConnectionStrings:MySql:worlds:{world}:data[]` to `worldConf.mysql.log[]` | N log shard MySQL instances per world | yes | `log` |

The runner must treat these as separate migration scopes even when local development points multiple connection strings at the same physical database.

`unified-data` does not exist in the current Pulumi or C# configuration yet. It is reserved for planned non-world sharded data. When it is introduced, it must not be folded into `unified`, because a single unified database and sharded unified data databases have different connection enumeration, locking, and ownership rules.

## Migration Scopes

Migrations are organized by target scope. A SQL file belongs to exactly one scope.

```text
infra/db/migrations/
  unified/
    20260630220000_baseline.sql
    20260701000100_add_marketplace_index.sql
  unified-data/
    20260701000150_add_shared_data_table.sql
  world-global/
    20260630220000_baseline.sql
    20260701000200_add_name_registry_index.sql
  world-data/
    20260630220000_baseline.sql
    20260701000300_add_item_column.sql
  world-log-data/
    20260630220000_baseline.sql
    20260701000400_add_log_index.sql
```

File naming:

```text
yyyyMMddHHmmss_snake_case_description.sql
```

Rules:

- Use UTC timestamp or a team-agreed local timestamp consistently.
- Never change a migration after it has been applied outside a local throwaway database.
- Add a new migration for every schema change.
- Baseline migrations use the `_baseline` suffix in the file name (for example `20260630220000_baseline.sql`). The runner treats any migration whose name is `baseline` as the scope baseline.
- Keep one behavioral change per migration file when possible.
- Use `-- @transaction off` only for MySQL statements that cannot safely run in a transaction.

## Baseline From `latest.sql`

`infra/db/latest.sql` contains one full schema dump. It includes database creation, destructive table drops, table definitions, and stored procedures.

For migration, it must be converted into scope-specific baseline files:

- Remove `DROP DATABASE`, `CREATE DATABASE`, and `USE`.
- Remove every `DROP TABLE IF EXISTS`.
- Keep `CREATE TABLE` statements only in the owning scope.
- Keep stored procedures only in the owning scope.
- Avoid environment-specific `DEFINER` clauses where possible. Prefer `CREATE PROCEDURE` without `DEFINER`, or normalize definer handling in the runner.
- Preserve charset/collation and indexes from `latest.sql`.

Baseline behavior:

- Fresh empty DB: apply the scope baseline normally.
- Existing DB without migration history: detect that baseline tables already exist, validate required sentinel objects, and record the baseline migration as applied.
- Existing DB with partial schema: fail startup and require manual repair. Do not guess.

Suggested sentinel validation:

| Scope | Sentinel Objects |
|---|---|
| `unified` | `marketplace_listing`, `marketplace_purchase` |
| `unified-data` | none until the first owner table is introduced |
| `world-global` | `name_registry`, `clan_name`, `USP_NAME_SET` |
| `world-data` | `user`, `item`, `mail`, `USP_MAIL_WRITE` |
| `world-log-data` | `log` |

## Current `latest.sql` Scope Mapping

The following mapping is the initial ownership proposal based on current code access patterns. If a table is moved to another physical DB later, its migration scope must move with the code that owns it.

### `unified`

Marketplace data is currently accessed through `ConnectionStrings:MySql:unified`.

Tables:

- `marketplace_listing`
- `marketplace_listing_archive`
- `marketplace_purchase`
- `marketplace_statistics`

Stored procedures:

- none

### `unified-data`

This is a planned non-world sharded data scope. It does not exist in the current `latest.sql`, Pulumi configuration, or C# `DbContext` connection API.

Tables:

- none yet

Stored procedures:

- none yet

Notes:

- Add tables here only when they are both non-world and sharded.
- Do not place marketplace tables here unless marketplace is intentionally moved from the single `unified` database to sharded unified data.
- The first implementation should add a dedicated configuration path and connection enumeration API instead of overloading world `data[]`.
- Suggested future configuration path: `ConnectionStrings:MySql:unifiedData[]`.
- Suggested future connection API: `GetUnifiedDataConnection(int index)` and `GetUnifiedDataShardDbSize()`.

### `world-global`

World-global data is one database per world and is accessed through `GetGlobalConnection(world)`. It stores world-wide registries and data that must not be sharded by character id.

Tables:

- `name_registry`
- `clan_name`

Stored procedures:

- `USP_NAME_GET_ID`
- `USP_NAME_SET`
- `USP_CLAN_NAME_DELETE`

Notes:

- Some code reads clan display data through global paths, but `clan` and `clan_member` are currently keyed and cached through shard-aware repositories. Keep them in `world-data` unless the ownership is intentionally changed.

### `world-data`

World-data is the regular per-world shard set. Most game state belongs here.

Tables:

- `achievement`
- `ban`
- `bulletin`
- `bulletin_sequence`
- `character_realtime_state`
- `clan`
- `clan_member`
- `group`
- `item`
- `mail`
- `mail_sequence`
- `marketplace_pending`
- `option`
- `quest`
- `spell`
- `storage_box`
- `system_mail`
- `system_storage_box`
- `user`
- `marriage`
- `write_back_failure`

Stored procedures:

- `USP_BULLETIN_ADD`
- `USP_BULLETIN_DELETE`
- `USP_BULLETIN_UPDATE`
- `USP_BULLETIN_GET`
- `USP_BULLETIN_GET_LIST`
- `USP_MAIL_GET_SUMMARY_LIST`
- `USP_MAIL_GET_LIST`
- `USP_MAIL_COUNT_BY_USER`
- `USP_MAIL_READ`
- `USP_MAIL_WRITE`
- `USP_MAIL_WRITE_MANY`
- `USP_MAIL_DELIVER_SYSTEM_MANY`

Notes:

- `write-back` consumes queued SQL and writes to `world-global` or `world-data`, but it should not own migrations.
- `write_back_failure` records write-back errors and belongs with the game DB scope because it includes world and shard context.

### `world-log-data`

Log data is deployed as separate log MySQL shards and is consumed by the `log` server.

Tables:

- `log`

Stored procedures:

- none

## Migration History Table

Every physical database gets its own history table:

```sql
CREATE TABLE IF NOT EXISTS `schema_migrations` (
  `scope` varchar(64) NOT NULL,
  `version` varchar(32) NOT NULL,
  `name` varchar(190) NOT NULL,
  `checksum` char(64) NOT NULL,
  `applied_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `applied_by` varchar(128) NOT NULL,
  PRIMARY KEY (`scope`, `version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

The checksum is SHA-256 of the normalized migration SQL content. If a version exists with a different checksum, startup must fail.

## Concurrency Control

Startup migration can run from multiple replicas. The runner must acquire a MySQL advisory lock per physical database and scope:

```sql
SELECT GET_LOCK('fb:migration:{scope}:{databaseFingerprint}', 300);
```

Rules:

- If lock acquisition times out, fail startup.
- Always release the lock in `finally`.
- `databaseFingerprint` should be computed from normalized host, port, database, and scope.
- Deduplicate physical targets before running migrations. This avoids repeated work in local development where `unified`, `global`, and `data[]` may point to the same database.

## Runner Flow

For each enabled server:

1. Read `Database:AutoMigration`.
2. If disabled, log and return.
3. Build a list of migration targets for the server profile.
4. Deduplicate targets by normalized connection string and scope.
5. For each target:
   1. Create the target database/schema if it does not exist.
   2. Open MySQL connection.
   3. Acquire advisory lock.
   4. Create `schema_migrations` if missing.
   5. Handle baseline detection.
   6. Read applied migrations.
   7. Verify checksums for applied migrations.
   8. Apply pending migrations in version order.
   9. Insert history rows.
   10. Release advisory lock.
6. If any migration fails, throw and stop server startup.

The database/schema creation step uses the database name from the connection string and runs `CREATE DATABASE IF NOT EXISTS`. This is required for local development because each migration scope uses its own database name.

## Server Profiles

Profiles define which scopes a server may migrate.

| Server | Profile | Scopes |
|---|---|---|
| `internal` | `Internal` | `world-global`, `world-data` |
| `marketplace` | `Marketplace` | `unified`; add `unified-data` only if marketplace owns planned non-world sharded tables |
| `log` | `Log` | `world-log-data` |
| `http` | `Http` | only if this standalone server is deployed; same as `internal` if it owns game APIs |
| `write-back` | none | no migration |
| `admin-tool` | none | no migration |

`admin-tool` and `write-back` should depend on schemas being migrated by the owning service or by an explicit migration job.

## Code Layout

Recommended shared C# code:

```text
server/http/Migration/
  DatabaseMigrationOptions.cs
  DatabaseMigrationRunner.cs
  MigrationProfile.cs
  MigrationScope.cs
  MigrationTarget.cs
  MigrationScriptProvider.cs
  MySqlMigrationLock.cs
```

Recommended SQL location:

```text
infra/db/migrations/{scope}/*.sql
```

If embedding SQL in each server assembly is preferred for deployment simplicity, include migration files in the owning server project with `EmbeddedResource`. Otherwise, copy `infra/db/migrations` to the output directory for each server image.

## Appsettings and Pulumi

Add this section to local and generated k8s appsettings:

```json
{
  "Database": {
    "AutoMigration": false
  }
}
```

Pulumi controls this with a single per-environment flag in the environment config (for example `infra/pulumi/develop.json`):

```json
{
  "database": {
    "autoMigration": true
  }
}
```

Each migration-owning Pulumi module reads `conf.database.autoMigration` and injects `Database:AutoMigration` into its generated ConfigMap `appsettings.k8s.json`:

- `internal.js`
- `marketplace.js`
- `log.js`

`admin-tool` and `write-back` do not read the flag because they are not migration owners. Production environments should set `autoMigration` to `false` (or omit the `database` block, which defaults to `false`).

Local development appsettings should not point every scope at one `fb` database. Use separate database names by scope:

- `fb-unified`
- `fb-1-global`
- `fb-1-data-0`
- `fb-1-data-1`
- `fb-1-data-2`
- `fb-1-log-0`

## Writing Future Migrations

When changing schema:

1. Decide the owning scope first.
2. Add one SQL file under that scope.
3. Make the SQL safe for repeated startup by relying on `schema_migrations`, not by silently ignoring every error.
4. Prefer additive changes:
   - `CREATE TABLE`
   - `ALTER TABLE ADD COLUMN`
   - `CREATE INDEX`
   - `CREATE OR REPLACE` compatible procedure updates, or explicit drop/create procedure inside a migration
5. For destructive changes:
   - Deploy code that stops using the old object first.
   - Run cleanup migration in a later release.
   - Document rollback limitations in the migration file header.

## Stored Procedure Handling

MySQL procedure migrations should use explicit drop/create in a single migration:

```sql
DROP PROCEDURE IF EXISTS `USP_NAME_GET_ID`;
DELIMITER //
CREATE PROCEDURE `USP_NAME_GET_ID`(...)
BEGIN
  ...
END //
DELIMITER ;
```

If the runner executes statements directly through `MySqlConnector`, it should not depend on `DELIMITER`, because `DELIMITER` is a mysql-client command. Either:

- store one procedure per file and let the runner execute the whole body as one command after parsing metadata, or
- use a simple script splitter that understands procedure blocks.

This needs to be implemented before moving stored procedures out of `latest.sql`.

## Open Decisions

- Whether migration SQL is embedded into server assemblies or copied as content into Docker images.
- Whether production enables startup migration or uses a dedicated one-shot migration deployment.
- Whether `clan` and `clan_member` should remain `world-data` or move to `world-global` for operational clarity.
- Whether local development should bootstrap empty DBs automatically from baseline or require a separate setup command.
