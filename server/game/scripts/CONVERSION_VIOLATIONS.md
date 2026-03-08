# Script Conversion Rule Violations

This document lists violations of `.cursor/rules/athena-to-lua-conversion.mdc` and `.cursor/rules/lua-standard.mdc` found in `server/game/scripts/`. Each item references the rule and affected files/lines.

**Fixed in this pass:** "주었습니다" messages (보약의달인, 랑구륜, 우선녀); label naming (나무꾼, 랑구륜); quest flow + reward-before-consume in 보약의달인; 통통대감/진진 (rmitem table, mkitem before rmitem, list sel/btn). Second pass: quest flow split (nil / step 0) in 병든아이어머니, 상해주민, 도재영, 고구려무도가, 소림사승려, 주경원, 먹염, 진백랑, 서천좌상, 원주민정령사, npc.lua; rmitem table format in 중국공주, 장안성장군, 남천좌상, 조왕의동생, 탕탕대감, 퉁퉁대감, 원주민정령사, 진백랑, 상해주민, 복건성태자, 천선도사, 산소소탕대장, 영채; 도성연 list 0-based (warp_sel == 0) and rmitem table + reward-before-consume; 도재영 rmitem table + reward-before-consume; list (sel, btn) in 고구려무도가, 주경원, 먹염, 진백랑, 서천좌상, 원주민정령사, npc.lua.

---

## 1. Quest flow: Do not combine `quest == nil` with `step == 0`

**Rule:** Section 1.1 — Use separate branches: (1) `quest == nil` = not started, (2) `quest:completed()` = done, (3) step-based. Do not add extra conditions like `quest == nil or quest:step() == 0`.

**Affected files:**

| File | Line | Current condition |
|------|------|-------------------|
| `npc/보약의달인.lua` | 10 | `quest == nil or quest:step() == 0` |
| `npc/병든아이어머니.lua` | 9 | `quest == nil or quest:step() == 0` |
| `npc/상해주민.lua` | 4 | `quest == nil or quest:step() == 0` |
| `npc/도재영.lua` | 86 | `quest == nil or quest:step() == 0` |
| `npc/고구려무도가.lua` | 5 | `quest == nil or quest:step() == 0` |
| `npc/소림사승려.lua` | 5 | `quest == nil or quest:step() == 0 or quest:step() >= 3` |
| `npc/주경원.lua` | 5 | `quest == nil or quest:step() == 0` |
| `npc/먹염.lua` | 33 | `quest == nil or quest:step() == 0` |
| `npc/진백랑.lua` | 9 | `quest == nil or quest:step() == 0` |
| `npc/서천좌상.lua` | 25 | `quest == nil or quest:step() == 0` |
| `npc/원주민정령사.lua` | 18 | `quest == nil or quest:step() == 0` |
| `npc.lua` | 694 | `quest == nil or quest:step() == 0` |

**Fix:** Split into: `if quest == nil then ... return end`, then `if quest:completed() then ... return end`, then step-based branches. Handle “intro / accept at step 0” in a dedicated `if quest:step() == 0 then ... end` block, not combined with `quest == nil`.

---

## 2. rmitem / has_items: Table only for 2+ item types

**Rule:** Section 3 — Use the **table** form only when **two or more** item types are involved. For a **single** item type, use the non-table form: `me:has_items(item_name, count)` and `me:rmitem(item_name, count, ITEM_DELETE_TYPE.GIVE)`.

**Single-item cases (non-table is preferred):** e.g. `npc/통통대감.lua` (망치), `npc/진진.lua` (초보도시락), `npc/중국공주.lua` (공주의반지), `npc/도재영.lua` (노비문서), `npc/소림사승려.lua` (고구려비단), `npc/먹염.lua` (food_name), etc. — keeping `me:rmitem('이름', n, GIVE)` is correct.

**Use table when:** two or more item types (e.g. `me:rmitem({ ['감초'] = 1, ['녹용'] = 1, ['국광'] = 1 }, GIVE)`).

---

## 3. "~ 주었습니다" message after rmitem with GIVE

**Rule:** Section 3 and lua-standard — When using `me:rmitem(..., ITEM_DELETE_TYPE.GIVE)`, do not show a separate "~ 주었습니다" message.

**Affected:**

- `npc/보약의달인.lua` L73–74: `me:message('국광 주었습니다', ...)` and `me:message('감초 주었습니다', ...)` — remove.
- `npc/랑구륜.lua` L43: `me:message(EIGHT_TRIGRAMS[i] .. ' 주었습니다', ...)` in loop — remove.
- `npc/우선녀.lua` L89, L105: `me:message('정화비서 주었습니다', ...)` and `me:message('숯의정화 주었습니다', ...)` — remove.

---

## 4. Goto label naming: Use `NPC_<npc_id>_<label>`

**Rule:** lua-standard — Use `::NPC_<npc_id>_<label>::` and `goto NPC_<npc_id>_<label>` to avoid collisions.

**Affected:**

- `npc/나무꾼.lua`: `::COS001::` / `goto COS001` (NPC 180) → `::NPC_180_COS001::` / `goto NPC_180_COS001`.
- `npc/랑구륜.lua`: `::POS00000::`, `::POS00001::`, `goto POS00000`; `::COS002::`, `goto COS002` (NPC 2) → `NPC_2_POS00000`, `NPC_2_POS00001`, `NPC_2_COS002`.

---

## 5. me:list — Two return values and 0-based index

**Rule:** Section 2 — `me:list` returns `(sel, btn)`. Handle `btn == DIALOG_RESULT.QUIT` and `PREV` first; `sel` is 0-based and may be nil on quit/prev.

**Affected:**

- Scripts that capture only one value, e.g. `local selected = me:list(...)` and then check only `selected`: `통통대감.lua`, `진진.lua`, `한상약.lua`, `중국공주.lua`, `도성연.lua`, `도재영.lua`, `남천좌상.lua`, `랑구륜.lua` (several `sel = me:list(...)` without `btn`).
- Scripts that treat selection as 1-based: e.g. `도성연.lua` uses `warp_sel == 1` for first option; rule says 0-based, so first option should be `sel == 0`.

**Fix:** Use `local sel, btn = me:list(...)`, handle `btn == QUIT` then `btn == PREV`, then use `sel` (0-based). Adjust any `sel == 1` for “first option” to `sel == 0`.

---

## 6. Item hand-in: has_items (first check), rmitem return (abuse defense), rmitem then mkitem

**Rule:** Section 3.1 — (1) **First check:** Use `has_items` to see if the player has the required items; if not, dialog and return (proceed gate). (2) **Second check (abuse defense):** When removing items, call `rmitem` **before** `mkitem` and **check `rmitem` return value**; if false, do not give the reward and return. (3) **Order:** has_items → (logic/dialogs) → rmitem (check return) → mkitem → quest/achievement → success dialogs.

**Affected:**

- `npc/통통대감.lua` L42–46: Keep has_items or equivalent gate; ensure order is `rmitem` then `mkitem`; check `rmitem` return and on failure return without reward.
- `npc/진진.lua` L22–23: Same — keep proceed gate; `rmitem` then `mkitem`; check `rmitem` return.
- `npc/보약의달인.lua` L71–78: Keep has_items; order `rmitem` (check return) then `mkitem`; remove "주었습니다" messages.

---

## 7. Item scripts and npc.lua

- **Item scripts** (`item/*.lua`): Many use `me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)` or `me:rmitem('name')`. If the rule “table format” applies to all rmitem usages, these should be aligned (e.g. `me:rmitem({ [item] = 1 }, REDUCE)`). Confirm with API before changing.
- **npc.lua**: Contains `quest == nil or quest:step() == 0` (L694) and various `rmitem(slot, count, ...)` / `rmitem(name, count)` usages; same rules apply where applicable.

---

## Summary

| # | Rule | Scope |
|---|------|--------|
| 1 | Quest flow (no nil + step 0) | 12 files |
| 2 | rmitem table format | Multiple NPCs + interaction.lua |
| 3 | No "주었습니다" after GIVE | 3 files |
| 4 | Label naming NPC_&lt;id&gt;_&lt;label&gt; | 나무꾼, 랑구륜 |
| 5 | list (sel, btn), 0-based | Several NPCs |
| 6 | has_items (1st) + rmitem return (abuse), rmitem then mkitem | 통통대감, 진진, 보약의달인 |
