local resp     = require("integration.response")
local protocol = require("integration.protocol")
local bot_diag = require("integration.lib.bot_diag")

local MESSAGE_DOOR_CLOSE  = "문을 닫았습니다."
local MESSAGE_DOOR_OPEN   = "문을 열었습니다."
local MESSAGE_DOOR_LOCK   = "문을 잠궜습니다."
local MESSAGE_DOOR_LOCKED = "문이 잠겼습니다."

local function expect_door_message(text)
    return function(packet)
        return packet.type == "STATE" and packet.text == text
    end
end

local function expect_door_toggle_message(packet)
    if packet.type ~= "STATE" then
        return false
    end
    return packet.text == MESSAGE_DOOR_CLOSE or packet.text == MESSAGE_DOOR_OPEN
end

local function log_bot_state(bot, label)
    bot_diag.dump(bot, "Door test:" .. label)
end

local function request_door_toggle(bot, label)
    log("debug", string.format("Door test: %s — door toggle click #1", label))
    local first = bot:request(resp.message, protocol.door(), expect_door_toggle_message)
    if first == false or first == nil then
        log("fatal", string.format(
            "Door test: %s — door toggle click #1 failed (no STATE close/open within timeout)",
            label))
        log_bot_state(bot, label .. " after fail#1")
        return false
    end

    log("debug", string.format("Door test: %s — door toggle click #1 ok: %s", label, first.text))

    local second_expected
    if first.text == MESSAGE_DOOR_CLOSE then
        second_expected = MESSAGE_DOOR_OPEN
    elseif first.text == MESSAGE_DOOR_OPEN then
        second_expected = MESSAGE_DOOR_CLOSE
    else
        log("fatal", string.format("Door test: %s — unexpected first toggle text: %s", label, first.text))
        log_bot_state(bot, label .. " unexpected_toggle")
        return false
    end

    log("debug", string.format("Door test: %s — door toggle click #2 expect: %s", label, second_expected))
    local second = bot:request(resp.message, protocol.door(), expect_door_message(second_expected))
    if second == false or second == nil then
        log("fatal", string.format(
            "Door test: %s — door toggle click #2 failed (expected %s)",
            label,
            second_expected))
        log_bot_state(bot, label .. " after fail#2")
        return false
    end

    log("debug", string.format("Door test: %s — door toggle click #2 ok: %s", label, second.text))
    return true
end

test_suite {
    name      = "Door Test",
    bot_count = 1,

    should_skip = function(_ctx)
        if localhost() then
            log("debug", "Door test skipped on local host (requires server transfer)")
            return true
        end
        return false
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot_diag.dump(bot, "Door test:start")

            log("debug", "Door test: moving to 국내성")
            -- Door geometry lives on the source map; do not enter an instance slot.
            local ok, err = pcall(function()
                bot:transfer(protocol.chat(false, "/맵이동 국내성 109 13"))
            end)
            if ok == false then
                log("fatal", "Door test: transfer to 국내성 109,13 failed: " .. tostring(err))
                bot_diag.dump(bot, "Door test:transfer_fail")
                return false
            end
            bot = ctx:bot(0)
            log_bot_state(bot, "after transfer")
            ctx:sleep(500)

            bot:direction("TOP")
            log_bot_state(bot, "after direction TOP")
            ctx:sleep(200)

            if request_door_toggle(bot, "door (109,13)") == false then
                return false
            end

            bot:create_item("파란열쇠", 1)
            log("debug", "Door test: creating 파란열쇠")

            local lock = bot:request(resp.message, protocol.item_active(0), expect_door_message(MESSAGE_DOOR_LOCK))
            if lock == false or lock == nil then
                log("fatal", "Door test: door lock message missing")
                bot_diag.dump(bot, "Door test:lock_fail")
                return false
            end

            log("debug", "Door test: dropping key")
            bot:drop_item(0, false)

            local locked = bot:request(resp.message, protocol.door(), expect_door_message(MESSAGE_DOOR_LOCKED))
            if locked == false or locked == nil then
                log("fatal", "Door test: door locked message missing")
                bot_diag.dump(bot, "Door test:locked_fail")
                return false
            end

            local looted = bot:request(resp.item_update, protocol.loot(false), function(packet)
                return string.find(packet.name, "파란열쇠", 1, true) ~= nil
            end)
            if looted == false or looted == nil then
                log("fatal", "Door test: failed to loot 파란열쇠")
                bot_diag.dump(bot, "Door test:loot_fail")
                return false
            end

            bot:map_move("국내성", 110, 13)
            log("debug", "Door test: moving to new door location")
            bot_diag.dump(bot, "Door test:after_map_move_110_13")

            if request_door_toggle(bot, "door (110,13)") == false then
                return false
            end
            local opened = bot:request(resp.message, protocol.item_active(0), expect_door_message(MESSAGE_DOOR_OPEN))
            if opened == false or opened == nil then
                log("fatal", "Door test: door open message missing at (110,13)")
                bot_diag.dump(bot, "Door test:open_fail")
                return false
            end

            return true
        end,
    },
}
