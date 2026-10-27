local spell_runner = require("integration.lib.spell_runner")
local protocol = require("integration.protocol")

local M = {}

local resp = require("integration.response")

local SPELL_NAME = "헬파이어"
local BASE_DELAY = 7
local WEAPON_DELAY_REDUCTION = 4

local function parse_cooldown_seconds(message)
    return tonumber(message:match("^(%d+)초"))
end

local function build_groups(shared)
    shared = shared or {}

    return {
        {
            spell = SPELL_NAME,
            cast_type = "TARGET",
            oid = function(caster) return caster:oid() end,
            position = function(caster) return shared.pos or caster:position() end,
            variants = {
                {
                    name = "첫시전",
                    response = resp.spell_buff,
                    pre = function(caster, _, state)
                        caster:setup_bot_stats(10000000, 1000, 10000000, 1000)
                        caster:chat("/쿨타임초기화")
                        state.pos = caster:position()
                    end,
                    condition = function(packet)
                        if packet.name == SPELL_NAME and packet.duration > 0 then return true end
                        return nil
                    end,
                    post = function(caster, _, _, packet)
                        caster:mp(1000)
                        shared.pos = caster:position()
                        shared.first_cooldown = packet.duration
                        return packet.duration == BASE_DELAY
                    end,
                },
                {
                    name = "쿨다운중시전",
                    response = resp.message,
                    condition = function(packet)
                        if packet.type == "STATE"
                            and packet.text:find("초 후에 사용할 수 있습니다", 1, true) ~= nil then
                            return true
                        end
                        return nil
                    end,
                    post = function(_, _, _, packet)
                        local remaining = parse_cooldown_seconds(packet.text)
                        if remaining == nil then return false end
                        return remaining > 0 and remaining <= shared.first_cooldown
                    end,
                },
                {
                    name = "쿨타임초기화",
                    cast = function(caster, _, _, state)
                        state.packet = caster:request(resp.spell_buff, protocol.chat(false, "/쿨타임초기화"),
                            function(packet)
                                return packet.name == SPELL_NAME and packet.duration == 0
                            end)
                        return state.packet ~= nil and state.packet ~= false
                    end,
                },
                {
                    name = "레벨업후시전",
                    response = resp.spell_buff,
                    pre = function(caster)
                        caster:level(99)
                        caster:create_item("용랑제구봉", 1)
                        if caster:equip(0) == false then return false end
                        shared.pos = caster:position()
                        return true
                    end,
                    condition = function(packet)
                        if packet.name == SPELL_NAME and packet.duration > 0 then return true end
                        return nil
                    end,
                    post = function(caster, _, _, packet)
                        caster:mp(1000)
                        shared.pos = caster:position()
                        return packet.duration == shared.first_cooldown - WEAPON_DELAY_REDUCTION
                    end,
                },
            },
        },
    }
end

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "SKILL COOLDOWN DELAY TEST STARTED")
    local option = require("integration.lib.option")
    if option.disable_pk_protect(caster) == false then
        log("fatal", "SKILL COOLDOWN DELAY TEST: failed to disable PK_PROTECT for caster")
        return false
    end
    local shared = {}
    if spell_runner.run_spell_groups(build_groups(shared), caster, nil) == false then
        return false
    end
    log("debug", "SKILL COOLDOWN DELAY TEST COMPLETED")
    return true
end

return M
