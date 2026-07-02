local resp     = require("integration.response")
local protocol = require("integration.protocol")

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

local function format_position(pos)
    if pos == nil then
        return "nil"
    end
    return string.format("(%s,%s)", tostring(pos[1]), tostring(pos[2]))
end

local function log_bot_state(bot, label)
    log("debug", string.format(
        "Door test [%s]: map=%s pos=%s oid=%s",
        label,
        tostring(bot:map()),
        format_position(bot:position()),
        tostring(bot:oid())))
end

local function request_door_toggle(bot, label)
    log("debug", string.format("Door test: %s — door toggle click #1", label))
    local first = bot:request(resp.message, protocol.door(), expect_door_toggle_message)
    if first == false or first == nil then
        log("debug", string.format(
            "Door test: %s — door toggle click #1 failed (no STATE close/open within timeout)",
            label))
        log_bot_state(bot, label .. " after fail")
        return false
    end

    log("debug", string.format("Door test: %s — door toggle click #1 ok: %s", label, first.text))

    local second_expected
    if first.text == MESSAGE_DOOR_CLOSE then
        second_expected = MESSAGE_DOOR_OPEN
    elseif first.text == MESSAGE_DOOR_OPEN then
        second_expected = MESSAGE_DOOR_CLOSE
    else
        log("debug", string.format("Door test: %s — unexpected first toggle text: %s", label, first.text))
        return false
    end

    log("debug", string.format("Door test: %s — door toggle click #2 expect: %s", label, second_expected))
    local second = bot:request(resp.message, protocol.door(), expect_door_message(second_expected))
    if second == false or second == nil then
        log("debug", string.format(
            "Door test: %s — door toggle click #2 failed (expected %s)",
            label,
            second_expected))
        log_bot_state(bot, label .. " after fail")
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

            log("debug", "Door test: moving to 국내성")
            bot:transfer(protocol.chat(false, "/맵이동 국내성 109 13"))
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

            bot:request(resp.message, protocol.item_active(0), expect_door_message(MESSAGE_DOOR_LOCK))

            log("debug", "Door test: dropping key")
            bot:drop_item(0, false)

            bot:request(resp.message, protocol.door(), expect_door_message(MESSAGE_DOOR_LOCKED))

            bot:request(resp.item_update, protocol.loot(false), function(packet)
                return string.find(packet.name, "파란열쇠", 1, true) ~= nil
            end)

            bot:map_move("국내성", 110, 13)
            log("debug", "Door test: moving to new door location")

            if request_door_toggle(bot, "door (110,13)") == false then
                return false
            end
            bot:request(resp.message, protocol.item_active(0), expect_door_message(MESSAGE_DOOR_OPEN))

            return true
        end,
    },
}
