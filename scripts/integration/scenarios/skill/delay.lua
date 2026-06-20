local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local function parse_cooldown_seconds(message)
    return tonumber(message:match("^(%d+)초"))
end

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "SKILL COOLDOWN DELAY TEST STARTED")

    caster:setup_bot_stats(10000000, 1000, 10000000, 1000)

    local spell_slot = caster:learn_spell("헬파이어")
    if spell_slot == 0xFF then
        return false
    end

    local pos = caster:position()
    caster:request(
        resp.update_internal,
        protocol.spell_cast("TARGET", spell_slot, "", caster:oid(), pos),
        function(packet)
            return packet.ch_mp == 0
        end)

    caster:mp(1000)

    local resp2 = caster:request(
        resp.message,
        protocol.spell_cast("TARGET", spell_slot, "", caster:oid(), pos),
        function(packet)
            return packet.type == "STATE"
                and packet.text:find("초 후에 사용할 수 있습니다", 1, true) ~= nil
        end)

    local first_cooldown = parse_cooldown_seconds(resp2.text)
    if first_cooldown == nil then
        return false
    end

    log("debug", "First cooldown time: " .. first_cooldown .. " seconds")
    caster:level(99)
    caster:create_item("용랑제구봉", 1)

    if caster:equip(0) == false then
        return false
    end

    caster:sleep(first_cooldown * 1000)

    pos = caster:position()
    caster:request(
        resp.update_internal,
        protocol.spell_cast("TARGET", spell_slot, "", caster:oid(), pos),
        function(packet)
            return packet.ch_mp == 0
        end)

    caster:mp(1000)

    local resp3 = caster:request(
        resp.message,
        protocol.spell_cast("TARGET", spell_slot, "", caster:oid(), pos),
        function(packet)
            return packet.type == "STATE"
                and packet.text:find("초 후에 사용할 수 있습니다", 1, true) ~= nil
        end)

    local second_cooldown = parse_cooldown_seconds(resp3.text)
    if second_cooldown == nil then
        return false
    end

    log("debug", "Second cooldown time: " .. second_cooldown .. " seconds")

    if second_cooldown ~= first_cooldown - 4 then
        return false
    end

    log("debug", "SKILL COOLDOWN DELAY TEST COMPLETED")
    return true
end

return M
