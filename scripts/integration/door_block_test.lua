local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local bot_diag    = require("integration.lib.bot_diag")
local script_step = require("integration.lib.script_step")

local MESSAGE_DOOR_CLOSE   = "문을 닫았습니다."
local MESSAGE_DOOR_OPEN    = "문을 열었습니다."
local MESSAGE_DOOR_BLOCKED = "문 사이에 누군가 있습니다."

local MAP_NAME = "국내성"
local OPENER_X = 109
local OPENER_Y = 13
local DOOR_X   = 109
local DOOR_Y   = 12
local ASIDE_X  = 108
local ASIDE_Y  = 14

local function fail(bot, message)
    log("fatal", "Door block test: " .. message)
    bot_diag.dump(bot, "Door block test:" .. message)
    return false
end

local function expect_state(texts)
    return function(packet)
        if packet.type ~= "STATE" then
            return false
        end
        for _, text in ipairs(texts) do
            if packet.text == text then
                return true
            end
        end
        return false
    end
end

local function toggle(bot, texts)
    local packet = bot:request(resp.message, protocol.door(), expect_state(texts))
    if packet == false or packet == nil then
        return nil
    end
    return packet.text
end

-- The opener holds 파란열쇠, so a door locked by an earlier test still toggles.
local function ensure_open(bot)
    local text = toggle(bot, { MESSAGE_DOOR_CLOSE, MESSAGE_DOOR_OPEN })
    if text == MESSAGE_DOOR_CLOSE then
        text = toggle(bot, { MESSAGE_DOOR_OPEN })
    end
    return text == MESSAGE_DOOR_OPEN
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function move_to(ctx, index, x, y)
    local bot = ctx:bot(index)
    local ok, err = pcall(function()
        bot:transfer(protocol.chat(false, string.format("/맵이동 %s %d %d", MAP_NAME, x, y)))
    end)
    if ok == false then
        return nil, err
    end
    return ctx:bot(index)
end

test_suite {
    name      = "Door Block Test",
    bot_count = 2,

    should_skip = function(_ctx)
        if localhost() then
            log("debug", "Door block test skipped on local host (requires server transfer)")
            return true
        end
        return false
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            bot:clear_inventory()
            bot:clear_all_spells()
        end
    end,

    scenarios = {
        -- A character on a door tile keeps the door from closing; a super-hidden admin does not.
        function(ctx)
            local opener, err = move_to(ctx, 0, OPENER_X, OPENER_Y)
            if opener == nil then
                return fail(ctx:bot(0), "opener transfer failed: " .. tostring(err))
            end
            local blocker
            blocker, err = move_to(ctx, 1, ASIDE_X, ASIDE_Y)
            if blocker == nil then
                return fail(ctx:bot(1), "blocker transfer failed: " .. tostring(err))
            end
            ctx:sleep(500)

            opener:create_item("파란열쇠", 1)
            opener:direction("TOP")
            ctx:sleep(200)

            if ensure_open(opener) == false then
                return fail(opener, "could not open the door")
            end

            blocker:map_move(MAP_NAME, DOOR_X, DOOR_Y)
            ctx:sleep(500)

            local text = toggle(opener, { MESSAGE_DOOR_BLOCKED, MESSAGE_DOOR_CLOSE })
            if text ~= MESSAGE_DOOR_BLOCKED then
                return fail(opener, "door closed over a character: " .. tostring(text))
            end

            local slot = blocker:learn_spell("잠복근무")
            if slot == 0xFF then
                return fail(blocker, "failed to learn 잠복근무")
            end
            blocker:send(protocol.spell_cast("NORMAL", slot, "", 0, { 0, 0 }))
            ctx:sleep(200)
            if run_step(blocker, "verify_super_hide", "door_hide_on", "1") == false then
                return fail(blocker, "잠복근무 did not hide")
            end

            -- A refused close would reveal where the hidden admin stands.
            text = toggle(opener, { MESSAGE_DOOR_BLOCKED, MESSAGE_DOOR_CLOSE })
            if text ~= MESSAGE_DOOR_CLOSE then
                return fail(opener, "door refused to close over a hidden admin: " .. tostring(text))
            end

            if toggle(opener, { MESSAGE_DOOR_OPEN }) ~= MESSAGE_DOOR_OPEN then
                return fail(opener, "could not reopen the door")
            end

            if run_step(blocker, "clear_super_hide", "door_hide_off") == false then
                return fail(blocker, "failed to clear 잠복근무")
            end
            blocker:map_move(MAP_NAME, ASIDE_X, ASIDE_Y)
            ctx:sleep(300)

            -- map:doors() returns every door once, even a door that spans several tiles.
            if run_step(opener, "verify_doors_unique", "doors_unique") == false then
                return fail(opener, "map:doors() returned the same door more than once")
            end

            return true
        end,
    },
}
