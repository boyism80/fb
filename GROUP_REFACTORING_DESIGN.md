# Group System Refactoring Design

## Current State Analysis

### Group Class Structure
- **Location**: `include/fb/game/group.h`, `server/game/lib/group.cpp`
- **Current State**:
  - Already has `server& _server` reference ✅
  - Has basic methods: `enter()`, `detach()`, `update()`, `id()`, `master()`, `members()`, `characters()`, `nears()`
  - Uses `std::vector<std::string>` for members (similar to clan's old structure)

### Server Class Group Methods
- **Location**: `include/fb/game/server.h`, `server/game/lib/server/server.group.cpp`
- **Current Methods**:
  - `create_group(character& me, const std::string& target)` - Returns `async::task<bool>`
  - `leave_group(character& me)` - Returns `async::task<void>`
  - `kick_group_member(const group& group, const std::string& kicker, const std::string& target)` - Returns `async::task<void>`
  - `broadcast(const group& group, const std::string& message, MESSAGE_TYPE type)` - Returns `async::task<void>`
  - `upsert_group_then()` - Two overloads (similar to clan's old `upsert_clan_then`)

### FlatBuffer Response Types
- **Location**: `protocol/internal.response.fbs`
- **Current Types**:
  - `GetGroup` - For querying group data
  - `EnterGroup` - Handles Create, Enter, Kick actions (with `GroupAction` enum)
  - `LeaveGroup` - Handles Leave, BreakUp actions (with `GroupAction` enum)
  - `KickGroup` - Handles Kick action (redundant with EnterGroup)
  - `BroadcastGroup` - For broadcasting messages

### AMQP Handlers
- **Location**: `server/game/lib/handler/amqp/`
- **Current Handlers**:
  - `enter_group` - Handles `EnterGroup` messages
  - `leave_group` - Handles `LeaveGroup` messages
  - `kick_group` - Handles `KickGroup` messages
  - No handler for `BroadcastGroup` (handled directly in `on_group_broadcast`)

### GroupAction Enum
- **Location**: `protocol/internal.fbs`
- **Current Values**:
  - `Create = 0`
  - `Enter = 1`
  - `Leave = 2`
  - `Kick = 3`
  - `BreakUp = 4`

### Builtin Functions
- **Location**: `server/game/lib/builtin/group.cpp`
- **Current Functions**:
  - `builtin_master` - Returns group master name
  - `builtin_members` - Returns group members list
  - `builtin_nears` - Returns nearby group members
  - `builtin_message` - Calls `server->broadcast(*group, ...)`
  - `builtin_kick` - Calls `server->kick_group_member(*group, ...)`

### GroupController
- **Location**: `server/internal/Controllers/GroupController.cs`
- **Current Endpoints**:
  - `GET /group/{id}` - Returns `Response.GetGroup`
  - `POST /group/create` - Returns `Response.EnterGroup` (with `GroupAction.Create` or `GroupAction.Enter`)
  - `POST /group/leave` - Returns `Response.LeaveGroup` (with `GroupAction.Leave` or `GroupAction.BreakUp`)
  - `POST /group/kick` - Returns `Response.KickGroup` (with `GroupAction.Kick`)
  - `POST /group/broadcast` - Returns `Response.BroadcastGroup`

## Comparison with Clan Refactoring

### Similarities
1. Both have `server&` reference in their classes
2. Both have similar method patterns (create, leave, kick, broadcast)
3. Both use FlatBuffer for protocol
4. Both have AMQP handlers for inter-server communication
5. Both have `host` field in responses (already implemented for group)

### Differences
1. **Response Type Structure**:
   - Clan: Unified `UpdatedClan` for most actions, separate `DestroyClan` for destruction, `ClanDetails` for query/create
   - Group: Separate `EnterGroup`, `LeaveGroup`, `KickGroup` (more fragmented)

2. **Action Handling**:
   - Clan: `ClanActionType` enum (Join, Leave, Kick, ChangeRole, SetTitle) - removed Create/Destroy
   - Group: `GroupAction` enum (Create, Enter, Leave, Kick, BreakUp) - all actions still present

3. **Member Data Type**:
   - Clan: Changed from `std::vector<clan_member>` to `std::unordered_map<std::string, clan_member>`
   - Group: Currently uses `std::vector<std::string>` (simpler, no role information)

4. **Method Parameters**:
   - Clan: Methods take `character&` for actors
   - Group: `kick_group_member` takes `const std::string& kicker` (should be `character&`)

5. **BreakUp vs Destroy**:
   - Clan: `Destroy` is a separate packet type (`DestroyClan`)
   - Group: `BreakUp` is handled within `LeaveGroup` response

## Proposed Refactoring Design

### 1. FlatBuffer Schema Changes

#### 1.1 New Response Types (Similar to Clan)
- **`GroupDetails`** (replaces `GetGroup` and used for Create):
  ```fbs
  table GroupDetails {
    host: uint;
    action: fb.protocol.internal.GroupDetailsAction;  // Query, Create
    group: fb.protocol.internal.Group;
    members: [fb.protocol.internal.CharacterRef];  // Use CharacterRef instead of string[]
    error: uint;
  }
  ```

- **`UpdatedGroup`** (unified response for Enter, Leave, Kick):
  ```fbs
  table UpdatedGroup {
    host: uint;
    action: fb.protocol.internal.GroupActionType;  // Enter, Leave, Kick (removed Create, BreakUp)
    group_id: uint;
    group_master: string;
    actor: fb.protocol.internal.CharacterRef;
    target: fb.protocol.internal.CharacterRef?;
    new_member: fb.protocol.internal.CharacterRef?;
    deleted_member: fb.protocol.internal.CharacterRef?;
    error: uint;
  }
  ```

- **`DestroyGroup`** (dedicated response for BreakUp):
  ```fbs
  table DestroyGroup {
    host: uint;
    group_id: uint;
    group_master: string;
    actor: fb.protocol.internal.CharacterRef;
    error: uint;
  }
  ```

- **`BroadcastGroup`** (remains separate, already has `host` field)

#### 1.2 New Enums
- **`GroupDetailsAction`**:
  ```fbs
  enum GroupDetailsAction : byte {
    Query = 0,
    Create = 1
  }
  ```

- **`GroupActionType`** (renamed from `GroupAction`, values re-indexed):
  ```fbs
  enum GroupActionType : byte {
    Enter = 0,
    Leave = 1,
    Kick = 2
  }
  ```

- **Remove `Create` and `BreakUp` from `GroupActionType`** (handled by `GroupDetailsAction::Create` and `DestroyGroup` respectively)

#### 1.3 Update `Group` Table
- Keep existing structure (id, master, members)
- Note: `members` is `[string]` in FlatBuffer, but we'll use `CharacterRef` in `GroupDetails`

### 2. Group Class Changes

#### 2.1 Member Data Type
- **Decision**: Keep `std::vector<std::string>` (groups don't have roles like clans)
- **Reason**: Groups are simpler than clans - no role hierarchy, just master and members

#### 2.2 New Methods (Move from Server)
- `async::task<void> enter_member(character& inviter, character& invitee)` - Replaces server's `create_group` logic
- `async::task<void> leave_member(character& leaver)` - Replaces server's `leave_group`
- `async::task<void> kick_member(character& kicker, character& target)` - Replaces server's `kick_group_member`
- `async::task<void> broadcast(const std::string& message, MESSAGE_TYPE type)` - Replaces server's `broadcast(const group&, ...)`

#### 2.3 Update Method Signature
- `update()` - Keep existing signature (takes `const std::string& master, const std::vector<std::string>& members`)

### 3. Server Class Changes

#### 3.1 Methods to Remove
- `kick_group_member(const group& group, const std::string& kicker, const std::string& target)`
- `broadcast(const group& group, const std::string& message, MESSAGE_TYPE type)`

#### 3.2 Methods to Keep
- `create_group(character& me, const std::string& target)` - Keep for now, but will call `group->enter_member()` internally
- `leave_group(character& me)` - Keep for now, but will call `group->leave_member()` internally
- `destroy_group()` - New method for handling BreakUp (similar to `destroy_clan`)

#### 3.3 Methods to Rename/Refactor
- `upsert_group_then()` → `ensure_group()` (similar to clan's `ensure_clan`)
- Update to use `GroupDetails` instead of `GetGroup`

#### 3.4 AMQP Handler Methods
- Remove: `on_enter_group()`, `on_leave_group()`, `on_kick_group()`
- Add: `on_updated_group(const internal_resp::UpdatedGroup& resp)`
- Add: `on_create_group(const internal_resp::GroupDetails& resp)` (for Create action)
- Add: `on_destroyed_group(const internal_resp::DestroyGroup& resp)` (for BreakUp action)
- Keep: `on_group_broadcast()` (unchanged)

### 4. AMQP Handler Changes

#### 4.1 Remove
- `enter_group.h/cpp`
- `leave_group.h/cpp`
- `kick_group.h/cpp`

#### 4.2 Add
- `create_group.h/cpp` - Handles `GroupDetails` with `GroupDetailsAction::Create`
- `updated_group.h/cpp` - Handles `UpdatedGroup` (Enter, Leave, Kick actions)
- `destroy_group.h/cpp` - Handles `DestroyGroup` (BreakUp action)

#### 4.3 Update
- `broadcast_group.h/cpp` - Already exists? (Need to verify)

### 5. GroupController Changes

#### 5.1 Endpoint Updates
- `GET /group/{id}` - Returns `Response.GroupDetails` with `Action = GroupDetailsAction.Query`
- `POST /group/create` - Returns `Response.GroupDetails` with `Action = GroupDetailsAction.Create` (publishes to RabbitMQ)
- `POST /group/leave` - Returns `Response.UpdatedGroup` (for Leave) or `Response.DestroyGroup` (for BreakUp) (publishes to RabbitMQ)
- `POST /group/kick` - Returns `Response.UpdatedGroup` with `Action = GroupActionType.Kick` (publishes to RabbitMQ)
- `POST /group/broadcast` - Returns `Response.BroadcastGroup` (unchanged)

#### 5.2 Response Construction
- Use `CharacterRef` instead of plain strings for member information in `GroupDetails`
- Add `host` field to all responses (already present in some)

### 6. Builtin Functions Changes

#### 6.1 Update Method Calls
- `builtin_message` - Change from `server->broadcast(*group, ...)` to `group->broadcast(...)`
- `builtin_kick` - Change from `server->kick_group_member(*group, ...)` to `group->kick_member(...)`
- Update to pass `character&` instead of `std::string` for kicker

### 7. Implementation Steps

#### Step 1: FlatBuffer Schema Updates
1. Add `GroupDetailsAction` enum to `protocol/internal.fbs`
2. Rename `GroupAction` to `GroupActionType` and remove `Create`, `BreakUp` values
3. Add `GroupDetails`, `UpdatedGroup`, `DestroyGroup` tables to `protocol/internal.response.fbs`
4. Update `Group` table if needed (keep as-is for now)
5. Regenerate FlatBuffer code

#### Step 2: Group Class Method Implementation
1. Add `enter_member()`, `leave_member()`, `kick_member()`, `broadcast()` methods to `group` class
2. Implement HTTP API calls using `_server.http`
3. Implement thread switching logic
4. Update `update()` method if needed (keep signature)

#### Step 3: Server Class Refactoring
1. Rename `upsert_group_then()` to `ensure_group()`
2. Update `ensure_group()` to use `GroupDetails` instead of `GetGroup`
3. Refactor `create_group()` to call `group->enter_member()` after group creation
4. Refactor `leave_group()` to call `group->leave_member()`
5. Add `destroy_group()` method for BreakUp handling
6. Remove old AMQP handler methods (`on_enter_group`, `on_leave_group`, `on_kick_group`)
7. Add new AMQP handler methods (`on_create_group`, `on_updated_group`, `on_destroyed_group`)

#### Step 4: AMQP Handler Implementation
1. Delete old handler files (`enter_group`, `leave_group`, `kick_group`)
2. Create new handler files (`create_group`, `updated_group`, `destroy_group`)
3. Implement `host` filtering logic in each handler
4. Update handler bindings in `server.cpp`

#### Step 5: GroupController Updates
1. Update `Get` endpoint to return `Response.GroupDetails`
2. Update `Create` endpoint to return `Response.GroupDetails` with `Action = Create`
3. Update `Leave` endpoint to return `Response.UpdatedGroup` or `Response.DestroyGroup`
4. Update `Kick` endpoint to return `Response.UpdatedGroup`
5. Add `host` field to all responses
6. Use `CharacterRef` in `GroupDetails` members

#### Step 6: Builtin Functions Updates
1. Update `builtin_message` to call `group->broadcast()`
2. Update `builtin_kick` to call `group->kick_member()` with `character&` parameters
3. Update Lua script calls if needed

#### Step 7: Testing and Verification
1. Test group creation flow
2. Test group member enter/leave/kick flows
3. Test group break up flow
4. Test group broadcast flow
5. Verify RabbitMQ message routing
6. Verify `host` field filtering

## Key Design Decisions

### 1. Member Data Type
- **Decision**: Keep `std::vector<std::string>` for group members
- **Reason**: Groups are simpler than clans - no role system, just master and members list

### 2. BreakUp Handling
- **Decision**: Use separate `DestroyGroup` packet (similar to clan's `DestroyClan`)
- **Reason**: Consistent with clan refactoring pattern, cleaner separation of concerns

### 3. Method Parameter Types
- **Decision**: Change `kick_group_member` to take `character&` instead of `std::string`
- **Reason**: Consistent with clan methods, enables thread switching

### 4. Response Type Unification
- **Decision**: Use `UpdatedGroup` for Enter, Leave, Kick actions
- **Reason**: Reduces code duplication, consistent with clan pattern

### 5. Create Action Handling
- **Decision**: Use `GroupDetails` with `GroupDetailsAction::Create` (similar to clan)
- **Reason**: Consistent with clan pattern, allows direct group construction from response

## Potential Issues and Considerations

### 1. Group ID vs Master ID
- **Current**: Group ID is the master's UID
- **Consideration**: This is different from clan (clan has separate ID). Keep as-is for now.

### 2. Member List Synchronization
- **Current**: `group::update()` handles member list updates
- **Consideration**: Ensure thread-safe updates when members join/leave

### 3. BreakUp vs Leave Logic
- **Current**: BreakUp is detected in `Leave` endpoint when master leaves
- **Consideration**: Ensure proper handling in `on_destroyed_group` vs `on_updated_group`

### 4. CharacterRef Usage
- **Current**: Groups use `std::string` for member names
- **Consideration**: `GroupDetails` should use `CharacterRef` for consistency, but internal representation can stay as `std::vector<std::string>`

## Summary

This refactoring will:
1. ✅ Unify group response types (similar to clan pattern)
2. ✅ Move group action methods from server to group class
3. ✅ Simplify AMQP handler structure
4. ✅ Improve consistency with clan system
5. ✅ Maintain existing functionality while improving code organization

The refactoring follows the same patterns established in the clan refactoring, ensuring consistency across the codebase.

