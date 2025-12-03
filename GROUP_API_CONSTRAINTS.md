# Group API Constraints Documentation

## Overview
This document describes the constraints, requirements, and error conditions for each Group API endpoint.

## Common Constraints

### Character Status Requirements
- **Actor (Master/Inviter/Kicker)**: Must be online (session exists)
- **Target (Member/Invitee)**: Must be online (session exists)
- **Map Validation**: Actor's map must exist in `Table.Map`

### Option Requirements
- **Group Option**: Both actor and target must have `Group` option enabled
  - Actor's `Group` option must be `true` (DisabledGroup error if false)
  - Target's `Member` option must be `true` (DisabledGroupTarget error if false)

### Self-Reference Prevention
- **CannotGroupSelf**: Actor and target cannot be the same character
  - Applies to: `Create`, `Enter`, `Kick`

### CharacterSync Requirements
- Both actor and target must have valid `CharacterSync` records
- Error: `NotFoundCharacterSync` if missing

---

## API Endpoints

### 1. GET `/group/{id}` - Get Group Details

**Purpose**: Query group information by group ID (master's UID)

**Request Parameters**:
- `id` (uint): Group ID (master's UID)

**Response**: `Response.GroupDetails` with `Action = Query`

**Constraints**:
- ✅ Group must exist (`GroupNotFound` error if missing)
- ✅ Master character must exist (`NotFoundCharacter` error if missing)
- ✅ All group members must exist (`NotFoundCharacter` error if any member missing)

**Error Codes**:
- `GroupNotFound`: Group with specified ID does not exist
- `NotFoundCharacter`: Master or any member character not found
- `Unhandled`: Unexpected error

**Notes**:
- Returns `GroupDetails` with all members as `CharacterRef` array
- Master is included in the members list
- `Group.Members` array is empty (members are in `Members` CharacterRef array)

---

### 2. POST `/group/create` - Create New Group

**Purpose**: Create a new group with actor as master and target as first member

**Request**: `Request.CreateGroup`
- `host` (uint): Game server ID
- `master` (uint): Actor's UID (will become group master)
- `member` (string): Target's character name (will become first member)

**Response**: `Response.GroupDetails` with `Action = Create`

**Constraints**:

#### Actor (Master) Requirements:
- ✅ Must be online (session exists)
- ✅ Must NOT be in any group (`GroupAlreadyJoined` error if already in group)
- ✅ Must have `Group` option enabled (`DisabledGroup` error if disabled)
- ✅ Map must be valid (`invalid map` exception if map not found)

#### Target (Member) Requirements:
- ✅ Must be online (`Offline` error if offline)
- ✅ Must NOT be in any group (`GroupTargetAlreadyJoined` error if already in group)
- ✅ Must have `Group` option enabled (`DisabledGroupTarget` error if disabled)
- ✅ Cannot be the same as actor (`CannotGroupSelf` error)

**Error Codes**:
- `CannotGroupSelf`: Actor and target are the same character
- `Offline`: Target character is offline
- `GroupAlreadyJoined`: Actor is already in a group
- `GroupTargetAlreadyJoined`: Target is already in a group
- `DisabledGroup`: Actor's group option is disabled
- `DisabledGroupTarget`: Target's group option is disabled
- `NotFoundCharacterSync`: Actor or target CharacterSync not found
- `Unhandled`: Unexpected error

**Behavior**:
- Creates new group with `Master = actor.Id`
- Adds target to `group.Members`
- Sets both `actorSync.Group` and `targetSync.Group` to `group.Master`
- Publishes `GroupDetails` response to RabbitMQ (`fb.group` exchange)

**Notes**:
- Group ID is the master's UID
- Only creates group if both actor and target are not in any group
- Automatically sets both characters' group references

---

### 3. POST `/group/enter` - Add Member to Existing Group

**Purpose**: Add a new member to an existing group (master invites new member)

**Request**: `Request.EnterGroup`
- `host` (uint): Game server ID
- `master` (uint): Group master's UID (actor)
- `member` (string): Target's character name (invitee)

**Response**: `Response.UpdatedGroup` with `Action = Enter`

**Constraints**:

#### Actor (Master) Requirements:
- ✅ Must be online (session exists)
- ✅ Must be in a group (`GroupNotJoined` error if not in group)
- ✅ Must be the group master (`NotGroupMaster` error if not master)
- ✅ Must have `Group` option enabled (`DisabledGroup` error if disabled)
- ✅ Map must be valid (`invalid map` exception if map not found)

#### Target (Member) Requirements:
- ✅ Must be online (`Offline` error if offline)
- ✅ Must NOT be in any group (`GroupTargetAlreadyJoined` error if already in group)
- ✅ Must have `Group` option enabled (`DisabledGroupTarget` error if disabled)
- ✅ Cannot be the same as actor (`CannotGroupSelf` error)
- ✅ Must not already be a member of the group (`GroupTargetAlreadyJoined` error if already member)

**Error Codes**:
- `CannotGroupSelf`: Actor and target are the same character
- `Offline`: Target character is offline
- `GroupNotJoined`: Actor is not in a group
- `NotGroupMaster`: Actor is not the group master
- `GroupTargetAlreadyJoined`: Target is already in a group or already a member
- `DisabledGroup`: Actor's group option is disabled
- `DisabledGroupTarget`: Target's group option is disabled
- `GroupNotFound`: Group does not exist
- `NotFoundCharacterSync`: Actor or target CharacterSync not found
- `Unhandled`: Unexpected error

**Behavior**:
- Adds target to `group.Members`
- Sets `targetSync.Group` to `group.Master`
- Publishes `UpdatedGroup` response to RabbitMQ (`fb.group` exchange)

**Notes**:
- Only group master can invite new members
- Target must not be in any group (including the target group)
- Checks both `targetSync.Group` and `group.Members` to prevent duplicates

---

### 4. POST `/group/leave` - Leave Group

**Purpose**: Remove a regular member from the group (non-master leaves)

**Request**: `Request.LeaveGroup`
- `host` (uint): Game server ID
- `member` (string): Character name leaving the group

**Response**: `Response.UpdatedGroup` with `Action = Leave`

**Constraints**:

#### Character Requirements:
- ✅ Must be online (`Offline` error if offline)
- ✅ Must be in a group (`GroupNotJoined` error if not in group)
- ✅ Must NOT be the group master (`NotGroupMaster` error if master - use `/group/destroy` instead)
- ✅ Must be a member of the group (`GroupNotJoined` error if not a member)
- ✅ Map must be valid (`NotFoundMap` error if map not found)

**Error Codes**:
- `Offline`: Character is offline
- `NotFoundCharacter`: Character not found
- `NotFoundMap`: Character's map not found
- `GroupNotJoined`: Character is not in a group or not a member
- `NotGroupMaster`: Character is the group master (must use `/group/destroy` instead)
- `GroupNotFound`: Group does not exist
- `NotFoundCharacterSync`: CharacterSync not found
- `Unhandled`: Unexpected error

**Behavior**:
- Removes character from `group.Members`
- Sets `sync.Group` to `null`
- Publishes `UpdatedGroup` response to RabbitMQ (`fb.group` exchange)

**Notes**:
- **Master cannot use this endpoint** - must use `/group/destroy` instead
- Only regular members can leave using this endpoint
- Group remains active after member leaves

---

### 5. POST `/group/kick` - Kick Member from Group

**Purpose**: Group master removes a member from the group

**Request**: `Request.KickGroup`
- `host` (uint): Game server ID
- `kicker` (string): Group master's character name (actor)
- `target` (string): Target member's character name (to be kicked)

**Response**: `Response.UpdatedGroup` with `Action = Kick`

**Constraints**:

#### Kicker (Master) Requirements:
- ✅ Must be online (`Offline` error if offline)
- ✅ Must be in a group (`GroupNotJoined` error if not in group)
- ✅ Must be the group master (`NotGroupMaster` error if not master)
- ✅ Map must be valid (`NotFoundMap` error if map not found)

#### Target Requirements:
- ✅ Must exist (`NotFoundCharacter` error if not found)
- ✅ Must be in the same group as kicker (`GroupNotJoined` error if different group)
- ✅ Must be a member of the group (`GroupNotJoined` error if not a member)
- ✅ Cannot be the same as kicker (`CannotGroupSelf` error)

**Error Codes**:
- `Offline`: Kicker is offline
- `NotFoundCharacter`: Kicker or target character not found
- `CannotGroupSelf`: Kicker and target are the same character
- `NotFoundMap`: Kicker's map not found
- `GroupNotJoined`: Kicker is not in a group, or target is not in the same group, or target is not a member
- `NotGroupMaster`: Kicker is not the group master
- `GroupNotFound`: Group does not exist
- `NotFoundCharacterSync`: Kicker or target CharacterSync not found
- `Unhandled`: Unexpected error

**Behavior**:
- Removes target from `group.Members`
- Sets `targetSync.Group` to `null`
- Publishes `UpdatedGroup` response to RabbitMQ (`fb.group` exchange)

**Notes**:
- Only group master can kick members
- Kicker and target must be in the same group
- Master cannot kick themselves (use `/group/destroy` instead)

---

### 6. POST `/group/destroy` - Destroy Group

**Purpose**: Group master destroys the entire group (removes all members)

**Request**: `Request.DestroyGroup`
- `host` (uint): Game server ID
- `master` (string): Group master's character name

**Response**: `Response.DestroyGroup`

**Constraints**:

#### Master Requirements:
- ✅ Must be online (`Offline` error if offline)
- ✅ Must be in a group (`GroupNotJoined` error if not in group)
- ✅ Must be the group master (`NotGroupMaster` error if not master)
- ✅ Map must be valid (`NotFoundMap` error if map not found)

**Error Codes**:
- `Offline`: Master is offline
- `NotFoundCharacter`: Master character not found
- `NotFoundMap`: Master's map not found
- `GroupNotJoined`: Master is not in a group
- `NotGroupMaster`: Master is not the group master
- `GroupNotFound`: Group does not exist
- `NotFoundCharacterSync`: Master or any member CharacterSync not found
- `NotFoundCharacter`: Any member character not found
- `Unhandled`: Unexpected error

**Behavior**:
- Removes all members from the group
- Sets all members' `CharacterSync.Group` to `null` (including master)
- Sets `group.Deleted = true`
- Publishes `DestroyGroup` response to RabbitMQ (`fb.group` exchange)

**Notes**:
- **Only group master can destroy the group**
- All members (including master) are removed from the group
- Group is marked as deleted but not physically removed from database
- All members' group references are cleared

---

### 7. POST `/group/broadcast` - Broadcast Message to Group

**Purpose**: Send a message to all group members

**Request**: `Request.BroadcastGroup`
- `host` (uint): Game server ID
- `group` (uint): Group ID (master's UID)
- `message` (string): Message to broadcast
- `type` (ubyte): Message type

**Response**: `Response.BroadcastGroup`

**Constraints**:
- ✅ Group must exist (`GroupNotFound` error if missing)

**Error Codes**:
- `GroupNotFound`: Group with specified ID does not exist
- `Unhandled`: Unexpected error

**Behavior**:
- Publishes `BroadcastGroup` response to RabbitMQ (`fb.group` exchange)
- Game servers receive the message via AMQP and broadcast to all group members

**Notes**:
- No character validation required (group existence only)
- Message delivery is handled by game servers via RabbitMQ
- All online group members receive the message

---

## Error Code Reference

| Error Code | Description | Affected APIs |
|------------|-------------|---------------|
| `CannotGroupSelf` | Actor and target are the same character | Create, Enter, Kick |
| `Offline` | Character is offline | Create, Enter, Leave, Kick, Destroy |
| `GroupAlreadyJoined` | Actor is already in a group | Create |
| `GroupTargetAlreadyJoined` | Target is already in a group or already a member | Create, Enter |
| `GroupNotJoined` | Character is not in a group or not a member | Enter, Leave, Kick, Destroy |
| `NotGroupMaster` | Character is not the group master | Enter, Kick, Destroy |
| `DisabledGroup` | Actor's group option is disabled | Create, Enter |
| `DisabledGroupTarget` | Target's group option is disabled | Create, Enter |
| `GroupNotFound` | Group does not exist | Get, Enter, Leave, Kick, Destroy, Broadcast |
| `NotFoundCharacter` | Character not found | All |
| `NotFoundCharacterSync` | CharacterSync not found | Create, Enter, Leave, Kick, Destroy |
| `NotFoundMap` | Map not found | Leave, Kick, Destroy |
| `Unhandled` | Unexpected error | All |

---

## State Transition Rules

### Group Creation Flow
1. Actor has no group → Create new group
2. Actor has group + is master → Enter new member
3. Actor has group + is master + target is member → Kick member

### Group Destruction Flow
1. Master leaves → Destroy group (all members removed)
2. Regular member leaves → Leave group (only that member removed)

### Permission Matrix

| Action | Master | Regular Member | Non-Member |
|--------|--------|----------------|------------|
| Create Group | ✅ | ❌ | ✅ |
| Enter Member | ✅ | ❌ | ❌ |
| Leave Group | ❌ (use Destroy) | ✅ | ❌ |
| Kick Member | ✅ | ❌ | ❌ |
| Destroy Group | ✅ | ❌ | ❌ |
| Broadcast | ✅ | ✅ | ❌ |

---

## Thread Safety and Locking

All group operations use distributed locks to ensure thread safety:

1. **CharacterSync Locks**: Lock both actor and target `CharacterSync` records
2. **Group Lock**: Lock the group record itself
3. **Lock Order**: Always acquire CharacterSync locks before Group lock to prevent deadlocks

**Lock Keys**:
- `CharacterSync.DistributedLockKey(characterId)`: Locks character's sync data
- `Group.DistributedLockKey(groupId)`: Locks group data

---

## RabbitMQ Message Publishing

All group operations (except `Get`) publish messages to RabbitMQ:
- **Exchange**: `amq.direct`
- **Routing Key**: `fb.group`
- **Purpose**: Notify all game servers of group state changes

**Message Types**:
- `GroupDetails`: Create action
- `UpdatedGroup`: Enter, Leave, Kick actions
- `DestroyGroup`: Destroy action
- `BroadcastGroup`: Broadcast action

**Host Field**: All responses include `host` field to identify the originating game server. Game servers ignore messages from their own `host` ID.

---

## Best Practices

1. **Always check group state before calling APIs**:
   - Check if actor has a group before calling `create` vs `enter`
   - Check if character is master before calling `destroy` vs `leave`

2. **Handle errors appropriately**:
   - `GroupAlreadyJoined`: Inform user they're already in a group
   - `NotGroupMaster`: Inform user they don't have permission
   - `GroupTargetAlreadyJoined`: Inform user target is already in a group

3. **Use correct endpoint for master operations**:
   - Master leaving → Use `/group/destroy` (not `/group/leave`)
   - Master inviting → Use `/group/enter` (not `/group/create`)

4. **Validate before API calls**:
   - Check if characters are online
   - Check if group options are enabled
   - Check if characters are already in groups

---

## Implementation Notes

### Game Server Logic
The game server's `create_group` method handles state checking and routes to appropriate APIs:
- No group → `/group/create`
- Has group + is master + target not in group → `/group/enter`
- Has group + is master + target in same group → `/group/kick`

### Response Handling
- `GroupDetails` (Create) → `on_create_group`
- `UpdatedGroup` (Enter, Leave, Kick) → `on_updated_group`
- `DestroyGroup` (Destroy) → `on_destroyed_group`
- `BroadcastGroup` → `on_group_broadcast`

