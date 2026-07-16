# Matchmaking Server — Implementation Design

## Summary

The matchmaking server is a standalone ASP.NET Core HTTP service, structurally aligned with `internal` and `marketplace`. It is **not world-scoped**: queue state and matching logic operate on a single global namespace.

Naming is fixed as **`MatchMaker`**, **`RegistryQueue`**, **`Registry`**, **`IRegistryEntry`**.

---

## Goals and Non-Goals

### Goals

- `MatchMaker.Register` and `RegistryQueue.TryFormMatch`
- Queue rules from config (`EntriesPerTeam`, `TeamsPerMatch`, tolerance, bucket width)
- Single-instance deployment

### Non-Goals (for now)

- Queue persistence / crash recovery
- Game server integration (RabbitMQ notify)

---

## Domain Model

```text
MatchMaker<TEntry> where TEntry : IRegistryEntry
  RegistryQueues: Dictionary<string, RegistryQueue<TEntry>>

RegistryQueue<TEntry>
  _byCreatedDate: SortedSet<Registry<TEntry>>              // global FIFO
  _buckets: SortedDictionary<int, List<Registry<TEntry>>> // key = bucket index

Registry<TEntry>
  Id: Guid
  CreatedDateTime: DateTime
  Entries: List<TEntry>

IRegistryEntry
  EntryId: string               // opaque unique id (format defined by concrete type)
  Mu: double
  Sigma: double

CharacterRegistryEntry : IRegistryEntry   // example concrete type
  World: uint
  CharacterId: uint
  Mu, Sigma
  EntryId => "{World}:{CharacterId}"
```

| Type | Meaning |
|------|---------|
| `IRegistryEntry` | Required: `EntryId`, `Mu`, `Sigma`. Core treats `EntryId` as an opaque string. |
| `CharacterRegistryEntry` | Example: `EntryId = "{World}:{CharacterId}"`; `World`/`CharacterId` are not used by matching logic. |
| `Registry<TEntry>` | Party registered together. Never split across teams. |
| `RegistryQueue<TEntry>` | Waiting queue for one mode/key. |
| `Match<TEntry>` | `TeamsPerMatch` teams; each team's entry count sums to `EntriesPerTeam`. |

`MatchMaker<TEntry>` is generic; the host registers a concrete entry type (currently `CharacterRegistryEntry` in `Program.cs`). The core only compares `EntryId` strings.

`CreatedDateTime` is set to `DateTime.UtcNow` in `MatchMaker.Register`.

### Entry identity vs API identity

- **Core**: `string EntryId` — opaque, compared as string equality.
- **HTTP / game layer**: `CharacterRegistryEntry` uses `"{World}:{CharacterId}"`; confirm / decline / status APIs parse or pass through that convention.

### Registry ordering (FIFO index)

`_byCreatedDate` orders by `CreatedDateTime` ascending, tie-break by `Id`.

---

## Skill Model (decided)

### Sigma aggregation — RSS

```text
RegistrySigma = sqrt( Σ entry.Sigma² )   over Registry.Entries

TeamSigma     = sqrt( Σ registry.RegistrySigma² )   over registries on the team
TeamMu        = entry-count weighted average of registry Mu
```

### Effective Mu — bucket assignment

```text
entryEffectiveMu  = entry.Mu - EffectiveMuSigmaFactor * entry.Sigma
EffectiveMu       = average(entryEffectiveMu) over Registry.Entries
bucketIndex       = floor(EffectiveMu / SkillBucketWidth)
```

Per-entry effective mu is averaged so party size does not shift bucket placement when members share the same individual rating. `ForRegistry` (aggregated sigma) is still used for team skill display.

- `EffectiveMuSigmaFactor` (k): default `3.0`
- `SkillBucketWidth`: default `1.0` (use `0.5` for finer grouping)

Bucket index is assigned once at enqueue and does not change while waiting.

**Bucket index is not Mu.** It is a sparse integer key; only indices with waiting registries exist in `_buckets`.

| EffectiveMu | SkillBucketWidth | bucketIndex |
|-------------|------------------|-------------|
| 26.3 | 1.0 | 26 |
| 26.7 | 1.0 | 26 |
| 26.3 | 0.5 | 52 |
| 26.7 | 0.5 | 53 |

Mu range for index `n`: `[n * width, (n + 1) * width)`.

### Per-registry tolerance

Tolerance is **not stored** on `Registry`. When a registry acts as anchor:

```text
anchorWaitSeconds = (UtcNow - anchor.CreatedDateTime).TotalSeconds
tolerance(anchor) = min(
    MaxSkillTolerance,
    BaseSkillTolerance + SkillTolerancePerSecond * anchorWaitSeconds
)
```

Tolerance is in **Mu space**. Bucket window is derived via `SkillBucketWidth`.

---

## Configuration

### Per-queue (`Queues` dictionary)

| Setting | Description | Typical value |
|---------|-------------|---------------|
| `EntriesPerTeam` | Entry count per team | `1`, `3`, … |
| `TeamsPerMatch` | Teams per match | `2` |

`EntriesPerMatch = EntriesPerTeam × TeamsPerMatch`.

### Global (`Matchmaking` section)

| Setting | Description | Default |
|---------|-------------|---------|
| `TickIntervalMs` | Background tick interval | `500` |
| `BaseSkillTolerance` | Initial bucket window half-width (Mu) | `1.0` |
| `SkillTolerancePerSecond` | Linear window growth per anchor wait second (Mu) | `0.1` |
| `MaxSkillTolerance` | Upper cap on tolerance (Mu) | `5.0` |
| `EffectiveMuSigmaFactor` | k in `effectiveMu = mu - k * sigma` | `3.0` |
| `SkillBucketWidth` | Effective-Mu span per bucket index | `1.0` |
| `ConfirmTimeoutSeconds` | Seconds for all entries to confirm a proposed match | `30` |

`SkillBucketWidth` must be `> 0`.

```json
{
  "Matchmaking": {
    "TickIntervalMs": 500,
    "BaseSkillTolerance": 1.0,
    "SkillTolerancePerSecond": 0.1,
    "MaxSkillTolerance": 5.0,
    "EffectiveMuSigmaFactor": 3.0,
    "SkillBucketWidth": 1.0,
    "ConfirmTimeoutSeconds": 30,
    "Queues": {
      "ranked-1v1": { "EntriesPerTeam": 1, "TeamsPerMatch": 2 },
      "ranked-3v3": { "EntriesPerTeam": 3, "TeamsPerMatch": 2 }
    }
  }
}
```

### Register validation

- `entries` non-empty; `entries.Count ≤ EntriesPerTeam`
- Each entry has valid `World`, `CharacterId`, `Mu`, `Sigma`
- No duplicate `EntryId` within the same `Register` request
- **Duplicate enrollment** rules (see below)
- Unknown `queueKey` rejected

---

## Enrollment Constraints (decided)

Exactly **one active matchmaking enrollment** per player. Structural rules:

| Rule | Constraint |
|------|------------|
| Player | Each `EntryId` belongs to **at most one** `Registry` at a time |
| Entry | Each `IRegistryEntry` belongs to **exactly one** `Registry<TEntry>` |
| Registry | Each `Registry` exists in **at most one** `RegistryQueue` at a time |
| Queue | A player cannot be enrolled in multiple queue keys simultaneously |

There is no concurrent waiting + pending, no duplicate `EntryId` across registries, and no registry shared across queues.

---

## Duplicate Enrollment (decided)

An `EntryId` may be enrolled **at most once** across all active matchmaking state on this server.

### Active state

| State | Counts as enrolled? |
|-------|---------------------|
| `Waiting` (in `RegistryQueue`) | Yes |
| `PendingConfirmation` (in `PendingMatchStore`) | Yes |
| Excluded / dissolved (fault) | No |
| Auto re-queued (`Requeue`) | Yes — immediately active again in queue |
| Finalized match (in progress) | No — game server owns player afterward |

### Rules

1. **`Register`**: reject if any entry's `EntryId` is already active (Waiting or Pending).
2. **Same request**: reject duplicate `EntryId` within `entries`.
3. **One registry per entry**: an `EntryId` cannot appear in two registries.
4. **One queue per registry**: a `Registry` cannot be in two `RegistryQueue` instances.
5. **Cross-queue**: one character cannot wait in `ranked-1v1` and `ranked-3v3` at the same time.

### Error

`AlreadyEnrolled` when `Register` conflicts with an existing `EntryId`.

### Index (implementation)

```text
_activeEntries: Dictionary<string, EnrollmentRef>   // key = EntryId
  EnrollmentRef → Waiting (queueKey, registryId) | Pending (matchId, registryId)

_registryIndex: Dictionary<Guid, (queueKey, state)>   // RegistryId → single location
```

Updated on register, unregister, pending create, dissolve, exclude, requeue.

---

## Exclude (decided)

**Exclude** means a registry (party) is **removed from matchmaking** and is **not** auto re-queued. Excluded registries **cannot** be enqueued until the game server sends a new `Register`.

### What exclude does

| Action | Yes / No |
|--------|----------|
| Remove from `RegistryQueue` / `PendingMatchStore` | Yes |
| Clear `EntryId` from `_activeEntries` | Yes |
| **Auto re-queue** | **No** |
| Manual `Register` again later | Yes (new enrollment) |

### When a registry is excluded

A registry is **excluded** if **any** of its entries:

- calls **`decline`**, or
- **fails to confirm before the deadline** (timeout)

**Even if other members of the same registry already confirmed**, one decliner or one timeout fails the **entire registry**. Those who confirmed are excluded together with their party.

### When a registry is auto re-queued

When a `PendingMatch` **dissolves** (decline or timeout), each **non-excluded** registry is **automatically re-enqueued** with **`CreatedDateTime` preserved**:

| Registry situation | Action |
|--------------------|--------|
| **All entries confirmed** before dissolution (caused by another registry) | **Auto re-queue** |
| **No entry declined / timed out** in this registry, but match dissolved early (another registry declined) | **Auto re-queue** (could not confirm due to others) |
| **Any entry declined or timed out** in this registry | **Exclude** — no re-queue |

Examples:

```text
Pending match: Registry A (party), Registry B (solo), Registry C (solo)

A: member 1 declines → A excluded (member 2 had confirmed — still excluded)
B: all confirmed      → auto re-queue
C: no confirm yet     → auto re-queue (dissolved because of A)

Timeout at deadline:
A: 1 of 2 confirmed → A excluded (partial confirm does not save the registry)
B: all confirmed    → auto re-queue (B ready, others failed)
C: no confirm       → C excluded (timeout — did not confirm in time)
```

### Decline dissolves the pending match

`POST /matchmaking/decline` is **required**. On decline:

1. Decliner's registry → **exclude**
2. `PendingMatch` → **dissolve**
3. Every other registry → classify with table above (**auto re-queue** or **exclude**)

### Exclude vs unregister

| | `unregister` | exclude |
|--|--------------|---------|
| Initiator | Voluntary leave while **Waiting** | Decline / timeout fault in registry |
| Auto re-queue | No | No |
| Manual `Register` again | Allowed | Allowed |

---

## Queue Storage — Effective-Mu Buckets

### Bucket index

```text
bucketIndex = floor(EffectiveMu / SkillBucketWidth)
```

Each bucket holds a `List<Registry>` in enqueue order (FIFO). The same registry also lives in `_byCreatedDate`.

Buckets are an **index**, not a fixed list of ranges and not rebuilt each tick:

- **Enqueue**: create bucket key if missing; append registry
- **Match attempt**: query index range from anchor + tolerance
- **Dequeue**: remove from both structures; delete empty bucket keys

---

## Tolerance and Bucket Window

For each anchor attempt:

```text
tolerance           = tolerance(anchor)              // Mu space
anchorEffectiveMu   = EffectiveMu(anchor)
minBucket           = floor((anchorEffectiveMu - tolerance) / SkillBucketWidth)
maxBucket           = floor((anchorEffectiveMu + tolerance) / SkillBucketWidth)
```

Candidates are all registries in buckets `[minBucket .. maxBucket]`, then sorted by `CreatedDateTime`, `Id`.

### Cross-bucket matching (decided)

**Bucket window is sufficient.** No additional team-balance or pairwise `Mu` check beyond the window.

---

## Selection Priority (decided)

| Stage | Criterion | Rule |
|-------|-----------|------|
| Anchor try order | Elapsed time | `_byCreatedDate` ascending (oldest first) |
| Bucket window | Effective Mu | `[minBucket .. maxBucket]` for current anchor |
| Team fill | Elapsed time | FIFO among candidates |

Do **not** sort candidates by Mu proximity.

---

## Match Formation

### BackgroundService

```text
every TickIntervalMs:
  foreach (queueKey, queue) in RegistryQueues:
    createdMatches = queue.TryFormMatch()
    foreach (match in createdMatches):
      dispatch(match)   // Phase 1: log only
```

### `TryFormMatch() → List<Match>`

```text
while total entry count >= EntriesPerMatch:

  match = null
  for anchor in _byCreatedDate (FIFO order):
    match = TryFormMatchWithAnchor(anchor)
    if match != null:
      break

  if match == null:
    break

  remove matched registries
  append match

return all matches formed this tick
```

### Anchor rotation (decided)

1. Try the **oldest** registry as anchor.
2. On failure, try the **next oldest** as anchor.
3. Continue until a match is formed or **every** registry has been tried.
4. If all fail → **0 matches** this round.

The longest-waiting player is **always tried first** each round, but may **not** be included when a later anchor succeeds in the same round. Their tolerance keeps growing on later ticks.

### `TryFormMatchWithAnchor(anchor)`

```text
1. tolerance = tolerance(anchor)
2. candidates = buckets [minBucket .. maxBucket], FIFO sorted
3. Build anchorTeam (must include anchor) to EntriesPerTeam
4. Build remaining teams from leftover candidates (FIFO backtracking)
5. Return Match or null
```

### Match rules

1. Each team has exactly `EntriesPerTeam` entries.
2. A match has exactly `TeamsPerMatch` teams.
3. `Registry` is atomic — never split across teams.
4. On formation, matched registries are **removed from the waiting queue** and moved to **pending confirmation** (see below).

---

## Match Confirmation (decided)

### Requirement

After a match is formed, **every `IRegistryEntry` (character)** must confirm within **n seconds** (`ConfirmTimeoutSeconds`, config).

| Outcome | Action |
|---------|--------|
| All registries: every entry confirms in time | Match **finalized** → notify game servers |
| Any registry: decline or timeout fault | `PendingMatch` **dissolved** → per-registry exclude or auto re-queue |

### Dissolution resolution (decided)

On dissolve (decline or timeout deadline), for **each registry** in the pending match:

```text
if registry has any declined entry OR any entry not confirmed by deadline:
  exclude registry
else:
  auto re-queue registry (CreatedDateTime preserved)
```

- Confirm tracked per **`EntryId`**; fault judgment per **`Registry`** (atomic party).
- **Innocent** registries (all members confirmed, or dissolved early without local fault) → **auto re-queue**.
- **Fault** registries (any decline or timeout in party) → **exclude**, including members who had already confirmed.

### Decline (decided)

`POST /matchmaking/decline` is **required**. Decline triggers immediate dissolution and exclude for the decliner's registry. See **Exclude** section.

### Config

| Setting | Description | Default |
|---------|-------------|---------|
| `ConfirmTimeoutSeconds` | Seconds to confirm after match creation | TBD (e.g. `30`) |

---

## Match Lifecycle

```text
[Waiting Queue]  --TryFormMatch-->  [Pending Confirmation]  --all confirm-->  [Finalized]
                                           |
                                           +-- dissolve -->  auto re-queue (innocent registries)
                                                         -->  exclude (fault registries)
```

### States

| State | Where stored | Description |
|-------|--------------|-------------|
| `Waiting` | `RegistryQueue` | In matchmaking queue |
| `PendingConfirmation` | `MatchMaker._pendingMatches` | Match proposed; awaiting entry confirms |
| `Finalized` | (transient) | All confirmed; dispatch to game then discard |
| `Dissolved` | — | Timeout; registries split into re-queue / exclude |

### `Match` (confirmation state)

```text
Match<TEntry>
  MatchId: Guid
  QueueKey: string
  CreatedAt: DateTime
  Teams: List<List<Registry<TEntry>>>
  ConfirmedEntryIds: HashSet<string>
```

Confirm deadline is derived at runtime: `CreatedAt + ConfirmTimeoutSeconds` (not stored on `Match`).
Pending matches are stored in `MatchMaker._pendingMatches` while awaiting player confirmation.

Participants are removed from `RegistryQueue` when moved to `PendingConfirmation`.

---

## Confirmation Flow — Implementation

### Components

| Component | Role |
|-----------|------|
| `MatchmakingBackgroundService` | Forms matches → `PendingMatchStore.Add` (no longer logs-only) |
| `MatchConfirmationBackgroundService` | Periodic scan for expired `Match`; apply re-queue / exclude |
| `MatchMaker` | Queues, `_pendingMatches`, `Register`, `Unregister`, `Requeue` |
| `MatchmakingController` | HTTP API (FlatBuffer) |

### Tick flow

```text
MatchmakingBackgroundService (existing tick):
  matches = queue.TryFormMatch()
  for each match:
    pending = PendingMatchStore.Create(match, queueKey)
    notify game servers (RabbitMQ — see Integration)

MatchConfirmationBackgroundService (new tick, e.g. every 1s):
  for each pending where UtcNow >= ConfirmDeadline:
    dissolve(pending, reason: Timeout)

dissolve(pending, reason):
  for each registry in pending:
    if registry has declined entry OR unconfirmed entry at deadline:
      exclude(registry)
    else:
      MatchMaker.Requeue(queueKey, snapshot, preserved CreatedDateTime)
  PendingMatchStore.Remove(pending.MatchId)

Decline API:
  mark declining EntryId
  dissolve(pending, reason: Decline)   // same per-registry logic; early deadline not required
```

### Confirm API flow

```text
POST /matchmaking/confirm
  1. Find PendingMatch by MatchId
  2. Verify (World, CharacterId) is a participant
  3. If UtcNow > ConfirmDeadline → reject (too late)
  4. Mark Confirmations[playerKey] = true
  5. If every `EntryId` in the match confirmed:
       finalize match → notify game servers
       PendingMatchStore.Remove
  6. Return success

POST /matchmaking/decline
  1. Find PendingMatch by MatchId
  2. Verify (World, CharacterId) is a participant
  3. Record decline for EntryId
  4. dissolve(pending, Decline) — exclude fault registries, auto re-queue innocent registries
  5. Return success
```

`finalize` = publish match-ready event (RabbitMQ / HTTP) with team roster; game server owns instance creation (open question).

### Concurrency

- `PendingMatchStore` and `MatchMaker` share a lock (or single coordinator service) for confirm + timeout to avoid races.
- Confirm arriving at deadline: compare `UtcNow` to `ConfirmDeadline` under the same lock as timeout handler.

### Elapsed time preservation

Auto re-queue uses the original `CreatedDateTime` from `RegistrySnapshot` (not `UtcNow`). Manual `Register` after exclude starts a new `CreatedDateTime`.

```csharp
MatchMaker.Requeue(queueKey, entries, preservedCreatedDateTime)
// New RegistryId, CreatedDateTime = preservedCreatedDateTime, re-enters queue + _activePlayers
```

---

## HTTP API (FlatBuffer)

Protocol namespace: `fb.protocol.matchmaking` (new; same pattern as `marketplace`).

Controller route prefix: `/matchmaking`.

### Endpoints

| Method | Path | Purpose |
|--------|------|---------|
| `POST` | `/matchmaking/register` | Enqueue a registry |
| `POST` | `/matchmaking/unregister` | Leave queue (waiting state only) |
| `POST` | `/matchmaking/confirm` | Confirm participation in a pending match |
| `POST` | `/matchmaking/decline` | Reject a pending match (required) |
| `POST` | `/matchmaking/status` | Query waiting / pending state for a character |

### Request / Response (draft)

#### `register`

```text
Request.Register
  QueueKey: string
  Entries: CharacterRegistryEntry[]   // World, CharacterId, Mu, Sigma

Response.Register
  RegistryId: Guid
  Error: uint
```

- Caller: game server (after player requests queue).
- Server sets `CreatedDateTime = UtcNow` on first register.

#### `unregister`

```text
Request.Unregister
  QueueKey: string
  RegistryId: Guid
  World: uint                // caller context (must match enrolled player)
  CharacterId: uint

Response.Unregister
  Success: bool
  Error: uint
```

- Only valid while registry is in **Waiting** state.
- Pending confirmation: use `decline` or wait for timeout.

#### `confirm`

```text
Request.Confirm
  MatchId: Guid
  World: uint
  CharacterId: uint

Response.Confirm
  Error: uint
  MatchFinalized: bool   // true when this confirm completed the last missing entry
```

#### `decline`

```text
Request.Decline
  MatchId: Guid
  World: uint
  CharacterId: uint

Response.Decline
  Error: uint
```

- Excludes the decliner's registry (and any registry with local fault).
- Dissolves match; **innocent** registries are **auto re-queued**.

#### `status`

```text
Request.Status
  World: uint
  CharacterId: uint

Response.Status
  InQueue: bool
  QueueKey: string
  RegistryId: Guid
  PendingMatchId: Guid    // empty if none
  ConfirmDeadline: DateTime
  Error: uint
```

### Error cases (draft)

| Code | When |
|------|------|
| Unknown queue key | Register |
| Already enrolled (`EntryId` active) | Register |
| Duplicate `EntryId` in same request | Register |
| Registry not found | Unregister |
| Not a participant | Confirm / Decline |
| Match not found / expired | Confirm / Decline |
| Already confirmed | Confirm |
| Already declined / dissolved | Decline |

---

## Game Server Integration (draft)

### Match proposed

When a match enters `_pendingMatches`, publish per involved world:

```text
exchange: amq.direct
routing: fb.{world}.matchmaking.proposed
payload: fb.protocol.matchmaking.mq.Proposed
```

### Match dissolved

When confirmation fails (decline or timeout):

```text
routing: fb.{world}.matchmaking.dissolved
payload: fb.protocol.matchmaking.mq.Dissolved
  reason: 0=Timeout, 1=Decline
  registry_outcomes[].outcome: 0=Excluded, 1=Requeued
```

### Match finalized

When all entries confirm:

```text
routing: fb.{world}.matchmaking.ready
payload: fb.protocol.matchmaking.mq.Ready
```

Game server creates arena / transfers players.

---

## Internal `MatchMaker<TEntry>` API (extended)

```csharp
Registry<TEntry> Register(string queueKey, IReadOnlyList<TEntry> entries)
bool Unregister(string queueKey, Guid registryId)
Registry<TEntry> Requeue(string queueKey, IReadOnlyList<TEntry> entries, DateTime preservedCreatedDateTime)
int GetQueueDepth(string queueKey)
```

`TEntry : IRegistryEntry`. Validation uses `entry.EntryId` (non-empty string) for duplicate detection within a registry.

`TryFormMatches()` remains internal to `MatchmakingBackgroundService`.

---

## Concurrency

- Single instance
- Per-`RegistryQueue` lock for `Add`, `Remove`, `TryFormMatch`

---

## Implementation Status

| Item | Status |
|------|--------|
| Project, `MatchMaker<TEntry>`, `IRegistryEntry`, models | Done |
| Effective Mu buckets + RSS `SkillCalculator` | Done |
| Configurable `SkillBucketWidth` | Done |
| Anchor rotation per tick | Done |
| Per-anchor tolerance with `MaxSkillTolerance` cap | Done |
| FIFO candidates + bucket window | Done |
| Match confirmation (`Match` + `_pendingMatches`) | Done |
| `MatchConfirmationBackgroundService` | Done |
| HTTP API (FlatBuffer) | Done |
| RabbitMQ (`proposed` / `dissolved` / `ready`) | Done |
| Docker / k8s | Done (`server/matchmaking/Dockerfile`, compose, k8s, Pulumi) |

---

## Open Questions (undecided)

### Product

1. **Queue key catalog** — Config-only vs datatable-driven?
2. **Who builds Registry** — Solo queue vs game `Group` party?
3. **Who creates the match instance** — Game server vs separate arena service?

### Operations

4. **Scaling** — Stay single-instance until needed; shard by queue key vs Redis-backed queue.

---

## Project Layout

```text
server/matchmaking/
  README.md
  matchmaking.csproj
  Program.cs
  appsettings.json
  Options/
  Model/
    IRegistryEntry.cs
    CharacterRegistryEntry.cs
    Registry.cs
    Match.cs
    Skill.cs
    SkillCalculator.cs
  Services/
    MatchMaker.cs
    RegistryQueue.cs
    MatchmakingBackgroundService.cs
    MatchConfirmationBackgroundService.cs
  Controllers/
    MatchmakingController.cs              // planned
  Formatter/
    FlatBufferFormatter.cs                // planned
```
