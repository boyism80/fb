-- Walks every dialog branch of the target NPCs from seeded states. A pool of bots takes
-- (NPC, seed) units from a shared queue in parallel.
-- Targets come from lib/npc_explorer_targets.lua (local-scripts/gen-explorer-targets.py).
-- Results are logged as EXPLORE|... / SUSPECT|..., written to npc-explorer-report.txt, and must
-- match lib/npc_explorer_expected.txt. Run from server\bot:
--   set FB_EXPLORE_NPCS=107,117   (optional: explore and compare only these NPCs)
--   set FB_EXPLORE_BOTS=16        (optional: bot pool size, at most 16)
--   set FB_EXPLORE_UPDATE=1       (accept the current results as the expected report)
--   bot.exe --mode integration --config config\config.dev.json --label explorer
local lib         = require("integration.lib")
local protocol    = require("integration.protocol")
local resp        = require("integration.response")
local qd          = require("integration.lib.quest_dialog")
local script_step = require("integration.lib.script_step")
local targets     = require("integration.lib.npc_explorer_targets")

local DIALOG_CLOSE_OID = 0xFFFFFFFD
local MAX_DEPTH        = 14
local MAX_PATHS        = 300
local LEVEL            = 50

-- target -> report lines in exploration order.
local g_report = {}
local g_bot_names = {}

-- Bot names are random per run and show up in NPC lines, so the report uses a placeholder.
local function normalize(text)
    for _, name in ipairs(g_bot_names) do
        text = text:gsub(name:gsub("%p", "%%%0"), "<bot>")
    end
    return text
end

local function emit(lines, line)
    line = normalize(line)
    log("info", line)
    table.insert(lines, line)
end

local function run(bot, func, step, ...)
    local pass, text = script_step.run_script(bot, "test/integration", func, step, ...)
    return pass == true, text
end

local function parse_snapshot(text)
    local q, m, map, i = (text or ""):match("q=([^;]*);m=(%-?%d+);map=(%d+);i=(.*)$")
    if q == nil then
        return nil
    end
    local snap = { quests = {}, money = tonumber(m), map = tonumber(map), items = {} }
    for id, state in q:gmatch("(%d+):([^,]+)") do
        snap.quests[id] = state
    end
    for name, count in i:gmatch("([^,=]+)=(%d+)") do
        snap.items[name] = tonumber(count)
    end
    return snap
end

local function snapshot(bot, quests)
    local _, text = run(bot, "snapshot", "snap", table.concat(quests, ","))
    return parse_snapshot(text)
end

-- Returns { quests = {...}, money = n, items = {...}, map = bool, text = string } or nil when nothing changed.
local function diff(before, after)
    local d = { quests = {}, items = {}, money = after.money - before.money, map = after.map ~= before.map }
    local parts = {}
    for id, state in pairs(after.quests) do
        if before.quests[id] ~= state then
            d.quests[id] = state
            table.insert(parts, string.format("q%s:%s>%s", id, before.quests[id], state))
        end
    end
    local names = {}
    for name in pairs(before.items) do names[name] = true end
    for name in pairs(after.items) do names[name] = true end
    for name in pairs(names) do
        local delta = (after.items[name] or 0) - (before.items[name] or 0)
        if delta ~= 0 then
            d.items[name] = delta
            table.insert(parts, string.format("%s%+d", name, delta))
        end
    end
    if d.money ~= 0 then
        table.insert(parts, string.format("money%+d", d.money))
    end
    if d.map then
        table.insert(parts, string.format("map:%d>%d", before.map, after.map))
    end
    if #parts == 0 then
        return nil
    end
    table.sort(parts)
    d.text = table.concat(parts, " ")
    return d
end

-- Something was received without paying money or items for it.
local function gained(d)
    if d == nil or d.money < 0 then
        return false
    end
    local received = d.money > 0
    for _, delta in pairs(d.items) do
        if delta < 0 then
            return false
        end
        received = received or delta > 0
    end
    return received
end

local function quest_changed(d)
    return d ~= nil and next(d.quests) ~= nil
end

-- The server runs the NPC script up to its next dialog, or to its end, before it handles the
-- next packet, so every dialog caused by the last action has arrived once self_info is answered.
-- Returns the new seq and the latest dialog, or seq and nil when the script sent none.
local function next_dialog(bot, seq)
    bot:request(resp.internal_info, protocol.self_info(), function()
        return true
    end)
    local current, dialog = bot:last_dialog()
    if current == seq then
        return seq, nil
    end
    return current, dialog
end

local function signature(dialog)
    local options = dialog.list_lists or dialog.menu_menus or {}
    local items = {}
    for _, item in ipairs(dialog.item_items or {}) do
        table.insert(items, item.name or "")
    end
    return table.concat({ dialog.kind or "", dialog.type or "", dialog.message or "",
        table.concat(options, "|"), table.concat(items, "|"),
        tostring(dialog.normal_button_prev), tostring(dialog.normal_button_next) }, "#")
end

-- Each action is { label = string, request = function() -> request or nil }.
local function actions(dialog)
    local list = {}
    local function add(label, make)
        table.insert(list, { label = label, make = make })
    end

    if dialog.kind == "ext" then
        if dialog.type == "normal" then
            if dialog.normal_button_next or dialog.normal_button_prev ~= true then
                add("NEXT", function() return protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT") end)
            end
            if dialog.normal_button_prev then
                add("PREV", function() return protocol.dialog("NORMAL", 0, "", 0, 0, "", "PREV") end)
            end
            add("QUIT", function() return protocol.dialog("NORMAL", 0, "", 0, 0, "", "QUIT") end)
        elseif dialog.type == "list" then
            for i, text in ipairs(dialog.list_lists or {}) do
                add("L:" .. text, function() return protocol.dialog("LIST", 0, "", i, 0, "", "NEXT") end)
            end
            if dialog.list_button_prev then
                add("PREV", function() return protocol.dialog("LIST", 0, "", 0, 0, "", "PREV") end)
            end
            add("QUIT", function() return protocol.dialog("LIST", 0, "", 0, 0, "", "QUIT") end)
        elseif dialog.type == "input_ext" then
            add("IN:1", function() return protocol.dialog("INPUT_EXT", 2, "1", 0, 0, "", "NEXT") end)
            add("QUIT", function() return protocol.dialog("INPUT_EXT", 1, "", 0, 0, "", "QUIT") end)
        else
            add("CLOSE", function() return protocol.click(DIALOG_CLOSE_OID) end)
        end
    else
        if dialog.type == "menu" then
            for i, text in ipairs(dialog.menu_menus or {}) do
                add("M:" .. text, function() return protocol.dialog("MENU", 0, "", i, 0, "") end)
            end
        elseif dialog.type == "pursuit" then
            for _, text in ipairs(dialog.menu_menus or {}) do
                add("P:" .. text, function() return protocol.dialog("PURSUIT", 0, "", 0, 0, text) end)
            end
        elseif dialog.type == "item" then
            for _, item in ipairs(dialog.item_items or {}) do
                add("I:" .. item.name, function() return protocol.dialog("ITEM", 0, "", 0, 0, item.name) end)
            end
        elseif dialog.type == "buy" then
            for _, entry in ipairs(dialog.buy_entries or {}) do
                add("B:" .. entry.name, function() return protocol.dialog("BUY", 0, "", 0, 0, entry.name) end)
            end
        elseif dialog.type == "input" then
            add("IN:1", function() return protocol.dialog("INPUT", 0, "1", 0, 0, "") end)
        elseif dialog.type == "slot" then
            for i in ipairs(dialog.slot_slots or {}) do
                add("S:" .. i, function() return protocol.dialog("SLOT", 0, "", i, 0, "") end)
            end
        end
        add("CLOSE", function() return protocol.click(DIALOG_CLOSE_OID) end)
    end
    return list
end

local function path_text(path)
    local labels = {}
    for _, action in ipairs(path) do
        table.insert(labels, action.label)
    end
    return table.concat(labels, " > ")
end

local function short(text)
    text = normalize((text or ""):gsub("[\r\n]+", " "))
    if #text > 60 then
        text = text:sub(1, 60) .. ".."
    end
    return text
end

local function errors_in(messages)
    local errors = {}
    for _, text in ipairs(messages) do
        if text:find("LUAERR", 1, true) ~= nil then
            table.insert(errors, text)
        end
    end
    return errors
end

-- Clicks the NPC and replays path. Returns the dialog left open (nil when the script ended)
-- and the dialog that was open before the last action.
local function replay(bot, oid, path)
    local seq = bot:last_dialog()
    bot:send(protocol.click(oid))
    local dialog
    seq, dialog = next_dialog(bot, seq)
    local previous = nil
    for _, action in ipairs(path) do
        if dialog == nil then
            return nil, previous, false
        end
        previous = dialog
        bot:send(action.make())
        seq, dialog = next_dialog(bot, seq)
    end
    return dialog, previous, true
end

-- Snapshots the state and resets it to seed in one request, so the next path starts fresh.
local function snapshot_reset(bot, quests, seed)
    local quest_list = #quests > 0 and table.concat(quests, ",") or "-"
    local _, text = run(bot, "snapshot_reset", "snap", quest_list, seed)
    return parse_snapshot(text)
end

-- Records the end of path (dialog is what is left open, nil when the script ended) and leaves
-- the state reset to seed. Returns the result when the path changed the state.
local function finish_path(bot, target, oid, seed, start, path, dialog, previous, reached, lines)
    if dialog ~= nil then
        bot:send(protocol.click(DIALOG_CLOSE_OID))
    end
    local after = snapshot_reset(bot, target.quests, seed)
    local errors = errors_in(bot:take_messages())
    local change = after and diff(start, after)
    local ending = dialog == nil and ("end after: " .. short(previous and previous.message)) or ("open: " .. short(dialog.message))
    local text = path_text(path)

    if after == nil then
        emit(lines, string.format("SUSPECT|%s|%s|%s|snapshot failed", target.name, seed, text))
    end
    if reached == false then
        emit(lines, string.format("SUSPECT|%s|%s|%s|dialog closed before the path ended", target.name, seed, text))
    end
    for _, err in ipairs(errors) do
        emit(lines, string.format("SUSPECT|%s|%s|%s|%s", target.name, seed, text, err))
    end
    if change == nil then
        return nil
    end

    emit(lines, string.format("EXPLORE|%s|%s|%s|%s|%s", target.name, seed, text, change.text, ending))
    if gained(change) and quest_changed(change) == false then
        emit(lines, string.format("SUSPECT|%s|%s|%s|reward without a quest change: %s",
            target.name, seed, text, change.text))
    end

    -- Run the path twice from the seed; a second reward is suspicious.
    if gained(change) then
        local first_dialog = replay(bot, oid, path)
        if first_dialog ~= nil then
            bot:send(protocol.click(DIALOG_CLOSE_OID))
        end
        local first = snapshot(bot, target.quests)
        local again_dialog = replay(bot, oid, path)
        if again_dialog ~= nil then
            bot:send(protocol.click(DIALOG_CLOSE_OID))
        end
        local again = snapshot_reset(bot, target.quests, seed)
        bot:take_messages()
        local second = first and again and diff(first, again)
        if gained(second) then
            emit(lines, string.format("SUSPECT|%s|%s|%s|repeatable reward: first %s, second %s",
                target.name, seed, text, change.text, second.text))
        end
    end
    return { seed = seed, path = text, actions = path, change = change }
end

-- Sets seed and returns the snapshot of the fresh state, or nil after reporting the failure.
local function start_seed(bot, target, seed, lines)
    if run(bot, "set_state", "seed", seed) == false then
        emit(lines, string.format("SUSPECT|%s|%s|seed failed", target.name, seed))
        return nil
    end
    local start = snapshot(bot, target.quests)
    if start == nil then
        emit(lines, string.format("SUSPECT|%s|%s|snapshot failed", target.name, seed))
    end
    return start
end

-- Depth-first walk over the dialog tree. A path is replayed from the NPC click only for the
-- siblings left on the stack; the first child of every dialog is taken in place.
local function explore_seed(bot, target, oid, seed, results, lines)
    local start = start_seed(bot, target, seed, lines)
    if start == nil then
        return
    end

    local visited = {}
    local paths = 0
    local stack = { {} }
    while #stack > 0 and paths < MAX_PATHS do
        local path = table.remove(stack)
        paths = paths + 1

        bot:take_messages()
        local dialog, previous, reached = replay(bot, oid, path)
        local limited = false
        while dialog ~= nil and #path < MAX_DEPTH and visited[signature(dialog)] == nil do
            visited[signature(dialog)] = true
            local next_actions = actions(dialog)
            for i = #next_actions, 2, -1 do
                local child = { table.unpack(path) }
                table.insert(child, next_actions[i])
                table.insert(stack, child)
            end
            if paths >= MAX_PATHS then
                limited = true
                break
            end

            paths = paths + 1
            path = { table.unpack(path) }
            table.insert(path, next_actions[1])
            previous = dialog
            local seq = bot:last_dialog()
            bot:send(next_actions[1].make())
            seq, dialog = next_dialog(bot, seq)
        end

        if limited then
            bot:send(protocol.click(DIALOG_CLOSE_OID))
        else
            local result = finish_path(bot, target, oid, seed, start, path, dialog, previous, reached, lines)
            if result ~= nil then
                table.insert(results, result)
            end
        end
    end
    if paths >= MAX_PATHS then
        emit(lines, string.format("SUSPECT|%s|%s|path limit %d reached", target.name, seed, MAX_PATHS))
    end
end

-- A full inventory only matters where the normal seed hands something out, so fill_seed replays
-- just the paths that gained something under the normal seed.
local function explore_fill_seed(bot, target, oid, fill_seed, normal_results, results, lines)
    local paths = {}
    for _, r in ipairs(normal_results) do
        if gained(r.change) then
            table.insert(paths, r.actions)
        end
    end
    if #paths == 0 then
        return
    end

    local start = start_seed(bot, target, fill_seed, lines)
    if start == nil then
        return
    end
    for _, path in ipairs(paths) do
        bot:take_messages()
        local dialog, previous, reached = replay(bot, oid, path)
        local result = finish_path(bot, target, oid, fill_seed, start, path, dialog, previous, reached, lines)
        if result ~= nil then
            table.insert(results, result)
        end
    end
end

-- A full inventory must not advance a quest the same way while dropping the reward.
local function compare_full_inventory(target, results, lines)
    local by_key = {}
    for _, r in ipairs(results) do
        by_key[r.seed .. "#" .. r.path] = r
    end
    for _, r in ipairs(results) do
        if r.seed:sub(-5) == ";fill" and quest_changed(r.change) then
            local normal = by_key[r.seed:sub(1, -6) .. "#" .. r.path]
            if normal ~= nil and gained(normal.change) and gained(r.change) == false then
                emit(lines, string.format("SUSPECT|%s|%s|%s|full inventory advanced the quest without the reward: full %s, normal %s",
                    target.name, r.seed, r.path, r.change.text, normal.change.text))
            end
        end
    end
end

local function selected_ids()
    local ids = {}
    local env = os.getenv("FB_EXPLORE_NPCS")
    if env ~= nil and env ~= "" then
        for id in env:gmatch("%d+") do
            table.insert(ids, tonumber(id))
        end
    else
        for id in pairs(targets) do
            table.insert(ids, id)
        end
        table.sort(ids)
    end
    return ids
end

local g_ids = selected_ids()

-- One work unit per (NPC, seed) so a large NPC spreads over several bots.
-- id -> { results = { [seed index] = {...} }, lines = { [seed index] = {...} }, remaining = n }
-- A ';fill' seed runs inside the unit of its normal seed, since it only replays that seed's paths.
local g_progress = {}
local g_units = {}
local g_unit_counts = {}
for _, id in ipairs(g_ids) do
    local target = targets[id]
    if target ~= nil then
        local index_of = {}
        for index, seed in ipairs(target.seeds) do
            index_of[seed] = index
        end
        local count = 0
        for index, seed in ipairs(target.seeds) do
            local normal = seed:sub(-5) == ";fill" and index_of[seed:sub(1, -6)] or nil
            if normal == nil then
                local fill = index_of[seed .. ";fill"]
                table.insert(g_units, { id = id, index = index, fill = fill })
                count = count + 1
            end
        end
        g_unit_counts[id] = count
    end
end

-- Scripts that pick their dialogs with math.random walk different paths on every run, so only
-- what their paths hand out and the errors they hit are compared.
local RANDOM_NPCS = { [392] = true, [476] = true }

local function outcome_lines(lines)
    local seen, outcome = {}, {}
    for _, line in ipairs(lines) do
        local kind, name, seed, rest = line:match("^(%u+)|([^|]*)|([^|]*)|(.*)$")
        local text = line
        if kind == "EXPLORE" then
            text = string.format("OUTCOME|%s|%s|%s", name, seed, rest:match("^[^|]*|([^|]*)|") or rest)
        elseif kind == "SUSPECT" then
            local reason = rest:match("^[^|]*|(.*)$") or rest
            if reason == "dialog closed before the path ended" then
                text = nil
            else
                text = string.format("SUSPECT|%s|%s|%s", name, seed, reason)
            end
        end
        if text ~= nil and seen[text] == nil then
            seen[text] = true
            table.insert(outcome, text)
        end
    end
    return outcome
end

-- Seed lines are kept per seed and joined in seed order once the NPC is done, so the report
-- does not depend on which bot finished first.
local function explore_unit(bot, unit)
    local target = targets[unit.id]
    local progress = g_progress[unit.id]
    if progress == nil then
        progress = { results = {}, lines = {}, remaining = g_unit_counts[unit.id] }
        g_progress[unit.id] = progress
    end

    local oid = qd.use_npc(bot, target.name)
    local results, lines = {}, {}
    explore_seed(bot, target, oid, target.seeds[unit.index], results, lines)
    progress.results[unit.index] = results
    progress.lines[unit.index] = lines
    if unit.fill ~= nil then
        local fill_results, fill_lines = {}, {}
        explore_fill_seed(bot, target, oid, target.seeds[unit.fill], results, fill_results, fill_lines)
        progress.results[unit.fill] = fill_results
        progress.lines[unit.fill] = fill_lines
    end
    qd.close(bot)
    progress.remaining = progress.remaining - 1
    if progress.remaining > 0 then
        return
    end

    local report = {}
    emit(report, string.format("TARGET|%d|%s|%d seeds", unit.id, target.name, #target.seeds))
    local all_results = {}
    for index in ipairs(target.seeds) do
        table.move(progress.lines[index], 1, #progress.lines[index], #report + 1, report)
        table.move(progress.results[index], 1, #progress.results[index], #all_results + 1, all_results)
    end
    compare_full_inventory(target, all_results, report)
    if RANDOM_NPCS[unit.id] then
        report = outcome_lines(report)
    end
    g_report[target] = report
end

-- TARGET|id|name|... names the NPC in the third field, every other line in the second.
local function line_target(line)
    local fields = {}
    for field in line:gmatch("[^|]+") do
        table.insert(fields, field)
    end
    if fields[1] == "TARGET" then
        return fields[3]
    end
    return fields[2]
end

local function read_lines(path)
    local file = io.open(path, "r")
    if file == nil then
        return nil
    end
    local lines = {}
    for line in file:lines() do
        if line ~= "" then
            table.insert(lines, line)
        end
    end
    file:close()
    return lines
end

local function write_lines(path, lines)
    local file = io.open(path, "w")
    if file == nil then
        return false
    end
    file:write(table.concat(lines, "\n"), "\n")
    file:close()
    return true
end

-- Compares the report with the expected report for the explored NPCs, or replaces their
-- expected lines when FB_EXPLORE_UPDATE=1.
local function verify_report()
    local explored = {}
    local actual = {}
    for _, id in ipairs(g_ids) do
        local target = targets[id]
        if target == nil then
            log("fatal", string.format("NPC explorer: no target %d in npc_explorer_targets", id))
            return false
        end
        explored[target.name] = true
        for _, line in ipairs(g_report[target] or {}) do
            table.insert(actual, line)
        end
    end
    write_lines("npc-explorer-report.txt", actual)

    local targets_path = package.searchpath("integration.lib.npc_explorer_targets", package.path)
    local expected_path = targets_path:gsub("npc_explorer_targets%.lua$", "npc_explorer_expected.txt")
    local stored = read_lines(expected_path) or {}

    if os.getenv("FB_EXPLORE_UPDATE") == "1" then
        local ids = {}
        for id in pairs(targets) do
            table.insert(ids, id)
        end
        table.sort(ids)

        local merged = {}
        for _, id in ipairs(ids) do
            local name = targets[id].name
            if explored[name] then
                for _, line in ipairs(g_report[targets[id]] or {}) do
                    table.insert(merged, line)
                end
            else
                for _, line in ipairs(stored) do
                    if line_target(line) == name then
                        table.insert(merged, line)
                    end
                end
            end
        end
        if write_lines(expected_path, merged) == false then
            log("fatal", "NPC explorer: cannot write " .. expected_path)
            return false
        end
        log("info", string.format("NPC explorer: expected report updated (%d lines)", #merged))
        return true
    end

    local remaining = {}
    for _, line in ipairs(stored) do
        if explored[line_target(line)] then
            remaining[line] = (remaining[line] or 0) + 1
        end
    end

    local mismatches = 0
    for _, line in ipairs(actual) do
        if (remaining[line] or 0) > 0 then
            remaining[line] = remaining[line] - 1
        else
            log("fatal", "NPC explorer: unexpected " .. line)
            mismatches = mismatches + 1
        end
    end
    for line, count in pairs(remaining) do
        for _ = 1, count do
            log("fatal", "NPC explorer: missing " .. line)
            mismatches = mismatches + 1
        end
    end

    if mismatches > 0 then
        log("fatal", string.format("NPC explorer: %d lines differ from %s (FB_EXPLORE_UPDATE=1 accepts the new report)",
            mismatches, expected_path))
        return false
    end
    return true
end

-- Each bot stands on its own tile with the tile below it free for the NPC it summons.
-- 낙랑의방 has blocked tiles at x 9-11, y 8-10, so the rows sit at y 6 and 12.
local HOME_MAP = "낙랑의방"
local HOME_COLUMNS = 8
local HOME_ROWS = { 6, 12 }
local function home(index)
    return 6 + index % HOME_COLUMNS, HOME_ROWS[math.floor(index / HOME_COLUMNS) + 1]
end

local MAX_BOTS = HOME_COLUMNS * #HOME_ROWS
local g_bot_count = tonumber(os.getenv("FB_EXPLORE_BOTS") or "") or MAX_BOTS
g_bot_count = math.max(math.min(g_bot_count, MAX_BOTS, #g_units), 1)
local g_next = 1

-- Lanes share one Lua state, so taking the next unit from the queue cannot race.
local g_lanes = {}
for index = 0, g_bot_count - 1 do
    g_lanes[index] = { function(ctx)
        local bot = ctx:bot(index)
        local x, y = home(index)
        bot:level(LEVEL)
        while g_next <= #g_units do
            local unit = g_units[g_next]
            g_next = g_next + 1
            -- A dialog may have warped the bot away; the summoned NPC stays below the home tile.
            bot:map_move(HOME_MAP, x, y, ctx:suite_slot())
            bot:direction("BOTTOM")
            explore_unit(bot, unit)
        end
        bot:map_move(HOME_MAP, x, y, ctx:suite_slot())
        qd.remove_npc(bot)
        bot:clear_inventory()
        return true
    end }
end

test_suite {
    name      = "NPC Dialog Explorer",
    bot_count = g_bot_count,

    on_initialize = function(ctx)
        g_next = 1
        g_progress = {}
        g_report = {}
        g_bot_names = {}
        for i = 0, ctx:bot_count() - 1 do
            table.insert(g_bot_names, ctx:bot(i):name())
        end
    end,

    scenarios = {
        { parallel = g_lanes },

        function()
            return verify_report()
        end,
    },
}
