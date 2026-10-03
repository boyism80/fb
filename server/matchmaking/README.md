# Matchmaking Server — Implementation Design

## Summary

The matchmaking server is a standalone ASP.NET Core HTTP service, structurally aligned with `internal` and `marketplace`. It is **not world-scoped**: queue state and matching logic operate on a single global namespace.

Naming is fixed as **`MatchMaker`**, **`TicketQueue`**, **`Ticket`**, **`ITicketMember`**.

---

## Goals and Non-Goals

### Goals

- `MatchMaker.EnqueueAsync` and `TicketQueue.TryFormMatch`
- Queue rules from config (`EntriesPerTeam`, `TeamsPerMatch`, tolerance, bucket width)
- Single-instance deployment

### Non-Goals (for now)

- Queue persistence / crash recovery
- Game server integration (RabbitMQ notify)

---

## Domain Model

```text
MatchMaker<TMember> where TMember : ITicketMember
  TicketQueues: Dictionary<string, TicketQueue<TMember>>

TicketQueue<TMember>
  _byCreatedDate: SortedSet<Ticket<TMember>>              // global FIFO
  _buckets: SortedDictionary<int, List<Ticket<TMember>>> // key = bucket index

Ticket<TMember>
  Id: ulong
  CreatedAt: DateTime
  Members: List<TMember>

ITicketMember
  MemberId: string               // opaque unique id (format defined by concrete type)
  Mu: double
  Sigma: double

CharacterTicketMember : ITicketMember   // example concrete type
  World: uint
  CharacterId: uint
  Mu, Sigma
  MemberId => "{World}:{CharacterId}"
```

| Type | Meaning |
|------|---------|
| `ITicketMember` | Required: `MemberId`, `Mu`, `Sigma`. Core treats `MemberId` as an opaque string. |
| `CharacterTicketMember` | Example: `MemberId = "{World}:{CharacterId}"`; `World`/`CharacterId` are not used by matching logic. |
| `Ticket<TMember>` | Party registered together. Never split across teams. |
| `TicketQueue<TMember>` | Waiting queue for one mode/key. |
| `Match<TMember>` | `TeamsPerMatch` teams; each team's member count sums to `EntriesPerTeam`. |

`MatchMaker<TMember>` is generic; the host registers a concrete member type (currently `CharacterTicketMember` in `Program.cs`). The core only compares `MemberId` strings.

`CreatedAt` is set to `DateTime.UtcNow` in `MatchMaker.EnqueueAsync`.

### Member identity vs API identity

- **Core**: `string MemberId` — opaque, compared as string equality.
- **HTTP / game layer**: `CharacterTicketMember` uses `"{World}:{CharacterId}"`; confirm / decline / status APIs parse or pass through that convention.

### Ticket ordering (FIFO index)

`_byCreatedDate` orders by `CreatedAt` ascending, tie-break by `Id`.

---

## Skill Model (decided)

### Sigma aggregation — RSS

```text
TicketSigma = sqrt( Σ member.Sigma² )   over Ticket.Members

TeamSigma     = sqrt( Σ ticket.TicketSigma² )   over tickets on the team
TeamMu        = member-count weighted average of ticket Mu
```

### Effective Mu — bucket assignment

```text
entryEffectiveMu  = member.Mu - EffectiveMuSigmaFactor * member.Sigma
EffectiveMu       = average(entryEffectiveMu) over Ticket.Members
bucketIndex       = floor(EffectiveMu / SkillBucketWidth)
```

Per-member effective mu is averaged so party size does not shift bucket placement when members share the same individual rating. `ForTicket` (aggregated sigma) is still used for team skill display.

- `EffectiveMuSigmaFactor` (k): default `3.0`
- `SkillBucketWidth`: default `1.0` (use `0.5` for finer grouping)

Bucket index is assigned once at enqueue and does not change while waiting.

**Bucket index is not Mu.** It is a sparse integer key; only indices with waiting tickets exist in `_buckets`.

| EffectiveMu | SkillBucketWidth | bucketIndex |
|-------------|------------------|-------------|
| 26.3 | 1.0 | 26 |
| 26.7 | 1.0 | 26 |
| 26.3 | 0.5 | 52 |
| 26.7 | 0.5 | 53 |

Mu range for index `n`: `[n * width, (n + 1) * width)`.

### Per-ticket tolerance

Tolerance is **not stored** on `Ticket`. When a ticket acts as anchor:

```text
anchorWaitSeconds = (UtcNow - anchor.CreatedAt).TotalSeconds
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
| `EntriesPerTeam` | Member count per team | `1`, `3`, … |
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
| `ConfirmTimeoutSeconds` | Seconds for all members to confirm a proposed match | `30` |

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

### Enqueue validation

- `members` non-empty; `members.Count ≤ EntriesPerTeam`
- Each member has valid `World`, `CharacterId`, `Mu`, `Sigma`
- No duplicate `MemberId` within the same `Enqueue` request
- **Duplicate enrollment** rules (see below)
- Unknown `queueKey` rejected

---

## Enrollment Constraints (decided)

Exactly **one active matchmaking enrollment** per player. Structural rules:

| Rule | Constraint |
|------|------------|
| Player | Each `MemberId` belongs to **at most one** `Ticket` at a time |
| Member | Each `ITicketMember` belongs to **exactly one** `Ticket<TMember>` |
| Ticket | Each `Ticket` exists in **at most one** `TicketQueue` at a time |
| Queue | A player cannot be enrolled in multiple queue keys simultaneously |

There is no concurrent waiting + pending, no duplicate `MemberId` across tickets, and no ticket shared across queues.

---

## Duplicate Enrollment (decided)

An `MemberId` may be enrolled **at most once** across all active matchmaking state on this server.

### Active state

| State | Counts as enrolled? |
|-------|---------------------|
| `Queued` (in `TicketQueue`) | Yes |
| `PendingConfirmation` (in `PendingMatchStore`) | Yes |
| Excluded / dissolved (fault) | No |
| Auto re-queued (`Requeue`) | Yes — immediately active again in queue |
| Finalized match (in progress) | No — game server owns player afterward |

### Rules

1. **`Enqueue`**: reject if any member's `MemberId` is already active (Queued or Proposed).
2. **Same request**: reject duplicate `MemberId` within `members`.
3. **One ticket per member**: an `MemberId` cannot appear in two tickets.
4. **One queue per ticket**: a `Ticket` cannot be in two `TicketQueue` instances.
5. **Cross-queue**: one character cannot wait in `ranked-1v1` and `ranked-3v3` at the same time.

### Error

`AlreadyEnrolled` when `Enqueue` conflicts with an existing `MemberId`.

### Index (implementation)

```text
_memberTickets: Dictionary<string, TicketRef>   // key = MemberId
  TicketRef → Queued (queueKey, ticketId) | Proposed (matchId, ticketId)

```

Updated on enqueue, dequeue, pending create, dissolve, exclude, requeue.

---

## Exclude (decided)

**Exclude** means a ticket (party) is **removed from matchmaking** and is **not** auto re-queued. Excluded tickets **cannot** be enqueued until the game server sends a new `Enqueue`.

### What exclude does

| Action | Yes / No |
|--------|----------|
| Remove from `TicketQueue` / `PendingMatchStore` | Yes |
| Clear `MemberId` from `_memberTickets` | Yes |
| Publish `TicketRemoved` to the members' worlds | Yes (queued tickets only) |
| **Auto re-queue** | **No** |
| Manual `Enqueue` again later | Yes (new enrollment) |

### When a ticket is excluded

A ticket is **excluded** if **any** of its members:

- calls **`decline`**, or
- **fails to confirm before the deadline** (timeout)

**Even if other members of the same ticket already confirmed**, one decliner or one timeout fails the **entire ticket**. Those who confirmed are excluded together with their party.

### When a ticket is auto re-queued

When a `PendingMatch` **dissolves** (decline or timeout), each **non-excluded** ticket is **automatically re-enqueued** with **`CreatedAt` preserved**:

| Ticket situation | Action |
|--------------------|--------|
| **All members confirmed** before dissolution (caused by another ticket) | **Auto re-queue** |
| **No member declined / timed out** in this ticket, but match dissolved early (another ticket declined) | **Auto re-queue** (could not confirm due to others) |
| **Any member declined or timed out** in this ticket | **Exclude** — no re-queue |

Examples:

```text
Pending match: Ticket A (party), Ticket B (solo), Ticket C (solo)

A: member 1 declines → A excluded (member 2 had confirmed — still excluded)
B: all confirmed      → auto re-queue
C: no confirm yet     → auto re-queue (dissolved because of A)

Timeout at deadline:
A: 1 of 2 confirmed → A excluded (partial confirm does not save the ticket)
B: all confirmed    → auto re-queue (B ready, others failed)
C: no confirm       → C excluded (timeout — did not confirm in time)
```

### Decline dissolves the pending match

`POST /matchmaking/decline` is **required**. On decline:

1. Decliner's ticket → **exclude**
2. `PendingMatch` → **dissolve**
3. Every other ticket → classify with table above (**auto re-queue** or **exclude**)

### Exclude vs dequeue

| | `dequeue` | exclude |
|--|--------------|---------|
| Initiator | Voluntary leave while **Waiting** | Decline / timeout fault in ticket |
| Auto re-queue | No | No |
| Manual `Enqueue` again | Allowed | Allowed |

---

## Queue Storage — Effective-Mu Buckets

### Bucket index

```text
bucketIndex = floor(EffectiveMu / SkillBucketWidth)
```

Each bucket holds a `List<Ticket>` in enqueue order (FIFO). The same ticket also lives in `_byCreatedDate`.

Buckets are an **index**, not a fixed list of ranges and not rebuilt each tick:

- **Enqueue**: create bucket key if missing; append ticket
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

Candidates are all tickets in buckets `[minBucket .. maxBucket]`, then sorted by `CreatedAt`, `Id`.

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
  foreach (queueKey, queue) in TicketQueues:
    createdMatches = queue.TryFormMatch()
    foreach (match in createdMatches):
      dispatch(match)   // Phase 1: log only
```

### `TryFormMatch() → List<Match>`

```text
while total member count >= EntriesPerMatch:

  match = null
  for anchor in _byCreatedDate (FIFO order):
    match = TryFormMatchWithAnchor(anchor)
    if match != null:
      break

  if match == null:
    break

  remove matched tickets
  append match

return all matches formed this tick
```

### Anchor rotation (decided)

1. Try the **oldest** ticket as anchor.
2. On failure, try the **next oldest** as anchor.
3. Continue until a match is formed or **every** ticket has been tried.
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

1. Each team has exactly `EntriesPerTeam` members.
2. A match has exactly `TeamsPerMatch` teams.
3. `Ticket` is atomic — never split across teams.
4. On formation, matched tickets are **removed from the waiting queue** and moved to **pending confirmation** (see below).
5. On formation, one live cross server (Redis heartbeat) is picked at random and its id is embedded in the snowflake `MatchId`. No live cross server means no match is formed; tickets stay queued.

---

## Match Confirmation (decided)

### Requirement

After a match is formed, **every `ITicketMember` (character)** must confirm within **n seconds** (`ConfirmTimeoutSeconds`, config).

| Outcome | Action |
|---------|--------|
| All tickets: every member confirms in time | Match **finalized** → notify game servers |
| Any ticket: decline or timeout fault | `PendingMatch` **dissolved** → per-ticket exclude or auto re-queue |

### Dissolution resolution (decided)

On dissolve (decline or timeout deadline), for **each ticket** in the pending match:

```text
if ticket has any declined member OR any member not confirmed by deadline:
  exclude ticket
else:
  auto re-queue ticket (CreatedAt preserved)
```

- Confirm tracked per **`MemberId`**; fault judgment per **`Ticket`** (atomic party).
- **Innocent** tickets (all members confirmed, or dissolved early without local fault) → **auto re-queue**.
- **Fault** tickets (any decline or timeout in party) → **exclude**, including members who had already confirmed.

### Decline (decided)

`POST /matchmaking/decline` is **required**. Decline triggers immediate dissolution and exclude for the decliner's ticket. See **Exclude** section.

### Config

| Setting | Description | Default |
|---------|-------------|---------|
| `ConfirmTimeoutSeconds` | Seconds to confirm after match creation | TBD (e.g. `30`) |

---

## Match Lifecycle

```text
[Waiting Queue]  --TryFormMatch-->  [Pending Confirmation]  --all confirm-->  [Finalized]
                                           |
                                           +-- dissolve -->  auto re-queue (innocent tickets)
                                                         -->  exclude (fault tickets)
```

### States

| State | Where stored | Description |
|-------|--------------|-------------|
| `Queued` | `TicketQueue` | In matchmaking queue |
| `PendingConfirmation` | `MatchMaker._pendingMatches` | Match proposed; awaiting member confirms |
| `Finalized` | (transient) | All confirmed; dispatch to game then discard |
| `Dissolved` | — | Timeout; tickets split into re-queue / exclude |

### `Match` (confirmation state)

```text
Match<TMember>
  MatchId: ulong   // snowflake: timestamp | cross host id | sequence
  QueueKey: string
  CreatedAt: DateTime
  Teams: List<List<Ticket<TMember>>>
  ConfirmedMemberIds: HashSet<string>
```

Confirm deadline is derived at runtime: `CreatedAt + ConfirmTimeoutSeconds` (not stored on `Match`).
Pending matches are stored in `MatchMaker._pendingMatches` while awaiting player confirmation.

Participants are removed from `TicketQueue` when moved to `PendingConfirmation`.

---

## Confirmation Flow — Implementation

### Components

| Component | Role |
|-----------|------|
| `MatchmakingBackgroundService` | Forms matches → `PendingMatchStore.Add` (no longer logs-only) |
| `MatchConfirmationBackgroundService` | Periodic scan for expired `Match`; apply re-queue / exclude |
| `MatchMaker` | Queues, `_pendingMatches`, `Enqueue`, `Dequeue`, `Requeue` |
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
  for each ticket in pending:
    if ticket has declined member OR unconfirmed member at deadline:
      exclude(ticket)
    else:
      MatchMaker.Requeue(queueKey, snapshot, preserved CreatedAt)
  PendingMatchStore.Remove(pending.MatchId)

Decline API:
  mark declining MemberId
  dissolve(pending, reason: Decline)   // same per-ticket logic; early deadline not required
```

### Confirm API flow

```text
POST /matchmaking/confirm
  1. Find PendingMatch by MatchId
  2. Verify (World, CharacterId) is a participant
  3. If UtcNow > ConfirmDeadline → reject (too late)
  4. Mark Confirmations[playerKey] = true
  5. If every `MemberId` in the match confirmed:
       finalize match → notify game servers
       PendingMatchStore.Remove
  6. Return success

POST /matchmaking/decline
  1. Find PendingMatch by MatchId
  2. Verify (World, CharacterId) is a participant
  3. Record decline for MemberId
  4. dissolve(pending, Decline) — exclude fault tickets, auto re-queue innocent tickets
  5. Return success
```

`finalize` = publish match-ready event (RabbitMQ / HTTP) with team roster; game server owns instance creation (open question).

### Concurrency

- `PendingMatchStore` and `MatchMaker` share a lock (or single coordinator service) for confirm + timeout to avoid races.
- Confirm arriving at deadline: compare `UtcNow` to `ConfirmDeadline` under the same lock as timeout handler.

### Elapsed time preservation

Auto re-queue uses the original `CreatedAt` from `TicketSnapshot` (not `UtcNow`). Manual `Enqueue` after exclude starts a new `CreatedAt`.

```csharp
MatchMaker.Requeue(queueKey, members, preservedCreatedAt)
// New TicketId, CreatedAt = preservedCreatedAt, re-enters queue + _activePlayers
```

---

## HTTP API (FlatBuffer)

Protocol namespace: `fb.protocol.matchmaking` (new; same pattern as `marketplace`).

Controller route prefix: `/matchmaking`.

### Endpoints

| Method | Path | Purpose |
|--------|------|---------|
| `POST` | `/matchmaking/enqueue` | Enqueue a ticket |
| `POST` | `/matchmaking/dequeue` | Leave queue (waiting state only) |
| `POST` | `/matchmaking/confirm` | Confirm participation in a pending match |
| `POST` | `/matchmaking/decline` | Reject a pending match (required) |
| `POST` | `/matchmaking/status` | Query waiting / pending state for a character |

### Request / Response (draft)

#### `enqueue`

```text
Request.Enqueue
  QueueKey: string
  Members: CharacterTicketMember[]   // World, CharacterId, Mu, Sigma

Response.Enqueue
  TicketId: ulong
  Error: uint
```

- Caller: game server (after player requests queue).
- Server sets `CreatedAt = UtcNow` on first enqueue.

#### `dequeue`

```text
Request.Dequeue
  QueueKey: string
  TicketId: ulong
  World: uint                // caller context (must match enrolled player)
  CharacterId: uint

Response.Dequeue
  Success: bool
  Error: uint
```

- Only valid while ticket is in **Waiting** state.
- Pending confirmation: use `decline` or wait for timeout.

#### `confirm`

```text
Request.Confirm
  MatchId: ulong
  World: uint
  CharacterId: uint

Response.Confirm
  Error: uint
  MatchFinalized: bool   // true when this confirm completed the last missing member
```

#### `decline`

```text
Request.Decline
  MatchId: ulong
  World: uint
  CharacterId: uint

Response.Decline
  Error: uint
```

- Excludes the decliner's ticket (and any ticket with local fault).
- Dissolves match; **innocent** tickets are **auto re-queued**.

#### `status`

```text
Request.Status
  World: uint
  CharacterId: uint

Response.Status
  InQueue: bool
  QueueKey: string
  TicketId: ulong
  PendingMatchId: ulong   // 0 if none
  ConfirmDeadline: DateTime
  Error: uint
```

### Error cases (draft)

| Code | When |
|------|------|
| Unknown queue key | Enqueue |
| Already enrolled (`MemberId` active) | Enqueue |
| Duplicate `MemberId` in same request | Enqueue |
| Ticket not found | Dequeue |
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
  ticket_outcomes[].outcome: 0=Excluded, 1=Requeued
```

### Match finalized

When all members confirm:

```text
routing: fb.{world}.matchmaking.ready
payload: fb.protocol.matchmaking.mq.Ready
```

Game server creates arena / transfers players.

---

## Internal `MatchMaker<TMember>` API (extended)

```csharp
Ticket<TMember> Enqueue(string queueKey, IReadOnlyList<TMember> members)
bool Dequeue(string queueKey, ulong ticketId)
Ticket<TMember> Requeue(string queueKey, IReadOnlyList<TMember> members, DateTime preservedCreatedAt)
int GetQueueDepth(string queueKey)
```

`TMember : ITicketMember`. Validation uses `member.MemberId` (non-empty string) for duplicate detection within a ticket.

`TryFormMatches()` remains internal to `MatchmakingBackgroundService`.

---

## Concurrency

- Single instance
- Per-`TicketQueue` lock for `Add`, `Remove`, `TryFormMatch`

---

## Implementation Status

| Item | Status |
|------|--------|
| Project, `MatchMaker<TMember>`, `ITicketMember`, models | Done |
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
2. **Who builds Ticket** — Solo queue vs game `Group` party?
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
    ITicketMember.cs
    CharacterTicketMember.cs
    Ticket.cs
    Match.cs
    Skill.cs
    SkillCalculator.cs
  Services/
    MatchMaker.cs
    TicketQueue.cs
    MatchmakingBackgroundService.cs
    MatchConfirmationBackgroundService.cs
  Controllers/
    MatchmakingController.cs              // planned
  Formatter/
    FlatBufferFormatter.cs                // planned
```
