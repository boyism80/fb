local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")

local function parse_cooldown_seconds(message)
    return tonumber(message:match("^(%d+)초"))
end

local function build_groups(shared)
    shared = shared or {}

    return {
        {
            spell = "헬파이어",
            cast_type = "TARGET",
            oid = function(caster) return caster:oid() end,
            position = function(caster) return shared.pos or caster:position() end,
            variants = {
                {
                    name = "첫시전",
                    response = resp.update_internal,
                    pre = function(caster, _, state)
                        caster:setup_bot_stats(10000000, 1000, 10000000, 1000)
                        state.pos = caster:position()
                    end,
                    condition = function(packet)
                        if packet.ch_mp == 0 then return true end
                        return nil
                    end,
                    post = function(caster)
                        caster:mp(1000)
                        shared.pos = caster:position()
                        return true
                    end,
                },
                {
                    name = "첫쿨다운",
                    response = resp.message,
                    condition = function(packet)
                        if packet.type == "STATE"
                            and packet.text:find("초 후에 사용할 수 있습니다", 1, true) ~= nil then
                            return true
                        end
                        return nil
                    end,
                    post = function(_, _, _, packet)
                        shared.first_cooldown = parse_cooldown_seconds(packet.text)
                        return shared.first_cooldown ~= nil
                    end,
                },
                {
                    name = "레벨업후시전",
                    response = resp.update_internal,
                    pre = function(caster)
                        caster:level(99)
                        caster:create_item("용랑제구봉", 1)
                        if caster:equip(0) == false then return false end
                        caster:sleep(shared.first_cooldown * 1000)
                        shared.pos = caster:position()
                        return true
                    end,
                    condition = function(packet)
                        if packet.ch_mp == 0 then return true end
                        return nil
                    end,
                    post = function(caster)
                        caster:mp(1000)
                        shared.pos = caster:position()
                        return true
                    end,
                },
                {
                    name = "두번째쿨다운",
                    response = resp.message,
                    condition = function(packet)
                        if packet.type == "STATE"
                            and packet.text:find("초 후에 사용할 수 있습니다", 1, true) ~= nil then
                            return true
                        end
                        return nil
                    end,
                    post = function(_, _, _, packet)
                        local second = parse_cooldown_seconds(packet.text)
                        if second == nil then return false end
                        return second == shared.first_cooldown - 4
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
