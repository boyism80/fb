-- Instance-map test (낙랑의방 + slot + front_info isolation).
--
-- Note:
-- - Client map id is model.id for both S and C, so bot:map_move / map_config
--   cannot distinguish instances. Isolation is asserted via ground drops + front_info.
-- - Instance enter/leave uses /맵이동 with an explicit slot (or without for S),
--   then sleeps briefly. Tighten waits when a better client signal exists.

local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local MAP          = "낙랑의방"
local SLOT         = 5
local POS          = {6, 6}
local DROP_POS     = {7, 6}
local MARKER_S     = "목도"
local MARKER_C     = "목검"
local MARKER_SHARE = "양첨목봉"
local ENTER_WAIT   = 1500
local DESTROY_WAIT = 3000

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil
        and message:find("AS EXPECTED", 1, true) == nil
        and message:find("SUCCESS:", 1, true) == nil then
        level = "fatal"
    end
    log(level, "[instance_map] " .. message)
    bot:chat("=== " .. message .. " ===")
end

local function fail(bot, message)
    progress(bot, "FAILED: " .. message)
    return false
end

local function move_to_source(bot, x, y)
    x = x or POS[1]
    y = y or POS[2]
    progress(bot, string.format("MOVE TO S %s (%d,%d)", MAP, x, y))
    -- Always chat: map_move early-outs when bot already thinks it is on this model.id.
    bot:chat(string.format("/맵이동 %s %d %d", MAP, x, y))
end

local function move_to_instance(bot, x, y, slot)
    x    = x or POS[1]
    y    = y or POS[2]
    slot = slot or SLOT
    progress(bot, string.format("MOVE TO C %s slot=%d (%d,%d)", MAP, slot, x, y))
    bot:chat(string.format("/맵이동 %s %d %d %d", MAP, x, y, slot))
end

local function drop_marker(bot, item_name)
    progress(bot, "DROP MARKER " .. item_name)
    bot:create_item(item_name, 1)
    bot:drop_item(0, false)
end

-- Collect STATE lines from one front_info until `done_fn(seen)` returns true.
-- Returns seen map, or nil on timeout/failure.
local function front_info_until(bot, done_fn, timeout_ms)
    timeout_ms = timeout_ms or 5000
    local seen = {}

    local result = bot:request(
        resp.message,
        protocol.front_info(),
        function(packet)
            if packet.type ~= "STATE" then
                return false
            end
            seen[packet.text] = true
            return done_fn(seen, packet.text) == true
        end,
        timeout_ms)

    if result == false then
        return nil
    end
    return seen
end

local function assert_front_contains(bot, item_name, label)
    progress(bot, string.format("%s: expect front_info '%s'", label, item_name))
    local seen = front_info_until(bot, function(seen)
        return seen[item_name] == true
    end)
    if seen == nil or seen[item_name] ~= true then
        return fail(bot, string.format("%s: front_info missing '%s'", label, item_name))
    end
    progress(bot, string.format("%s: front_info OK '%s'", label, item_name))
    return true
end

-- Expect to see present_name, and ensure absent_name is never reported.
local function assert_front_isolation(bot, present_name, absent_name, label)
    progress(bot, string.format("%s: expect '%s', not '%s'", label, present_name, absent_name))
    local seen = front_info_until(bot, function(seen)
        return seen[present_name] == true
    end)
    if seen == nil then
        return fail(bot, string.format("%s: front_info timeout (wanted '%s')", label, present_name))
    end
    if seen[absent_name] then
        return fail(bot, string.format("%s: still sees source marker '%s'", label, absent_name))
    end
    if seen[present_name] ~= true then
        return fail(bot, string.format("%s: missing instance marker '%s'", label, present_name))
    end
    progress(bot, string.format("%s: isolation OK", label))
    return true
end

local function cast_by_name(caster, target_name, spell_name)
    progress(caster, string.format("CAST %s -> %s", spell_name, target_name))
    caster:clear_all_spells()
    caster:learn_spells({spell_name})
    caster:set_current_hp_mp(10000, 10000)
    local expected_mp = caster:mp() - 30
    local result = caster:request(
        resp.update_internal,
        protocol.spell_cast("INPUT", 0, target_name, 0, {0, 0}),
        function(packet)
            return packet.ch_mp == expected_mp
        end,
        10000)
    return result ~= false
end

-- Stand at POS facing the drop tile (RIGHT -> DROP_POS).
local function face_drop_marker(ctx, bot, use_instance)
    if use_instance then
        move_to_instance(bot, POS[1], POS[2], SLOT)
    else
        move_to_source(bot, POS[1], POS[2])
    end
    ctx:sleep(ENTER_WAIT)
    bot:direction("RIGHT")
end

-- Clear ground items on S and on instance slot, then return to S.
local function clear_maps(ctx, bot)
    progress(bot, string.format("CLEAR GROUND ON S AND SLOT %d", SLOT))
    move_to_source(bot, POS[1], POS[2])
    ctx:sleep(ENTER_WAIT)
    bot:chat("/아이템삭제")
    ctx:sleep(300)

    move_to_instance(bot, POS[1], POS[2], SLOT)
    ctx:sleep(ENTER_WAIT)
    bot:chat("/아이템삭제")
    ctx:sleep(300)

    move_to_source(bot, POS[1], POS[2])
    ctx:sleep(DESTROY_WAIT)
end

test_suite {
    name      = "Instance Map Test",
    bot_count = 2,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "INSTANCE MAP TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
        progress(ctx:bot(0), "FORMATION DONE")
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            progress(bot, "CLEANUP")
            bot:chat("/아이템삭제")
            bot:clear_inventory()
            bot:clear_all_spells()
            bot:money(0)
        end
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)

            progress(a, string.format("SCENARIO START map=%s slot=%d", MAP, SLOT))

            a:setup_bot_stats(100000, 100000)
            b:setup_bot_stats(100000, 100000)
            a:level(50)
            b:level(40)
            a:clear_inventory()
            b:clear_inventory()

            clear_maps(ctx, a)
            move_to_source(b, POS[1] + 2, POS[2])
            ctx:sleep(ENTER_WAIT)

            -- 0) Baseline on source (S)
            progress(a, "STEP 0: baseline on S")
            a:chat(string.format("/맵이동 %s %d %d", MAP, DROP_POS[1], DROP_POS[2]))
            ctx:sleep(ENTER_WAIT)
            drop_marker(a, MARKER_S)
            face_drop_marker(ctx, a, false)

            if assert_front_contains(a, MARKER_S, "step0") == false then
                return false
            end

            -- 1) Ensure instance slot and enter (C)
            progress(a, string.format("STEP 1: enter instance slot %d", SLOT))
            move_to_instance(a, POS[1], POS[2], SLOT)
            ctx:sleep(ENTER_WAIT)

            -- 2) Isolation on C
            progress(a, "STEP 2: isolation on C")
            a:chat(string.format("/맵이동 %s %d %d %d", MAP, DROP_POS[1], DROP_POS[2], SLOT))
            ctx:sleep(ENTER_WAIT)
            drop_marker(a, MARKER_C)
            face_drop_marker(ctx, a, true)

            if assert_front_isolation(a, MARKER_C, MARKER_S, "step2") == false then
                return false
            end

            -- 3) B cross-check S then join C
            progress(b, "STEP 3: cross-check S then join C")
            face_drop_marker(ctx, b, false)
            if assert_front_contains(b, MARKER_S, "step3-S") == false then
                return false
            end

            face_drop_marker(ctx, b, true)
            if assert_front_isolation(b, MARKER_C, MARKER_S, "step3-C") == false then
                return false
            end

            -- 4) Shared C drop
            progress(a, "STEP 4: shared drop on C")
            a:chat(string.format("/맵이동 %s %d %d %d", MAP, DROP_POS[1], DROP_POS[2], SLOT))
            ctx:sleep(ENTER_WAIT)
            drop_marker(a, MARKER_SHARE)
            face_drop_marker(ctx, a, true)
            face_drop_marker(ctx, b, true)
            if assert_front_contains(b, MARKER_SHARE, "step4") == false then
                return false
            end

            -- 5) 소환 / 출두
            progress(a, "STEP 5: 소환 then 출두")
            move_to_source(b, POS[1] + 2, POS[2])
            ctx:sleep(ENTER_WAIT)
            move_to_instance(a, POS[1], POS[2], SLOT)
            ctx:sleep(ENTER_WAIT)

            if cast_by_name(a, b:name(), "소환") == false then
                return fail(a, "step5: 소환 failed")
            end
            ctx:sleep(1000)
            -- teleport_lookup places B beside A; re-align for front_info.
            face_drop_marker(ctx, b, true)
            if assert_front_contains(b, MARKER_SHARE, "step5-summon") == false then
                return false
            end

            move_to_source(a, POS[1], POS[2])
            ctx:sleep(ENTER_WAIT)
            if cast_by_name(a, b:name(), "출두") == false then
                return fail(a, "step5: 출두 failed")
            end
            ctx:sleep(1000)
            face_drop_marker(ctx, a, true)
            if assert_front_contains(a, MARKER_SHARE, "step5-chuldu") == false then
                return false
            end

            -- 6) Enter again while occupied
            progress(a, "STEP 6: re-enter occupied instance")
            move_to_source(a, POS[1], POS[2])
            ctx:sleep(ENTER_WAIT)
            face_drop_marker(ctx, a, true)
            if assert_front_contains(a, MARKER_SHARE, "step6") == false then
                return false
            end

            -- 7) Auto-destroy and recreate
            progress(a, "STEP 7: auto-destroy and recreate")
            move_to_source(a, POS[1], POS[2])
            move_to_source(b, POS[1] + 2, POS[2])
            ctx:sleep(DESTROY_WAIT)

            move_to_instance(a, POS[1], POS[2], SLOT)
            ctx:sleep(ENTER_WAIT)
            a:chat(string.format("/맵이동 %s %d %d %d", MAP, DROP_POS[1], DROP_POS[2], SLOT))
            ctx:sleep(ENTER_WAIT)
            drop_marker(a, MARKER_C)
            face_drop_marker(ctx, a, true)

            if assert_front_isolation(a, MARKER_C, MARKER_SHARE, "step7-recreate") == false then
                return false
            end

            face_drop_marker(ctx, a, false)
            if assert_front_contains(a, MARKER_S, "step7-source") == false then
                return false
            end

            progress(a, "CLEANUP MAPS")
            move_to_instance(a, POS[1], POS[2], SLOT)
            ctx:sleep(ENTER_WAIT)
            a:chat("/아이템삭제")
            move_to_source(a, POS[1], POS[2])
            move_to_source(b, POS[1] + 2, POS[2])
            ctx:sleep(ENTER_WAIT)
            a:chat("/아이템삭제")
            a:clear_inventory()
            b:clear_inventory()
            a:clear_all_spells()

            progress(a, "SCENARIO PASSED")
            return true
        end,
    },
}
