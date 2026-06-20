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

test_suite {
    name      = "Door Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)

            log("debug", "Door test: moving to 국내성")
            bot:transfer(protocol.chat(false, "/맵이동 국내성 109 13"))
            bot:direction("TOP")

            bot:request(resp.message, protocol.door(), expect_door_message(MESSAGE_DOOR_CLOSE))
            bot:request(resp.message, protocol.door(), expect_door_message(MESSAGE_DOOR_OPEN))

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

            bot:request(resp.message, protocol.door(), expect_door_message(MESSAGE_DOOR_CLOSE))
            bot:request(resp.message, protocol.door(), expect_door_message(MESSAGE_DOOR_OPEN))
            bot:request(resp.message, protocol.item_active(0), expect_door_message(MESSAGE_DOOR_OPEN))

            return true
        end,
    },
}
