local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "사자후전사",
        response = resp.message,
        cast_type = "INPUT",
        message = "전사 사자후 테스트 메시지입니다!",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 1000)
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.type == "SHOUT" then return true end
            return nil
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                error(string.format("사자후전사 MP failed: expected %d got %d", state.expected_mp, caster:mp()))
            end
            return true
        end,
    },
    {
        name = "사자후도사",
        response = resp.message,
        cast_type = "INPUT",
        message = "도사 사자후 테스트 메시지입니다!",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 1000)
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.type == "SHOUT" then return true end
            return nil
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                error(string.format("사자후도사 MP failed: expected %d got %d", state.expected_mp, caster:mp()))
            end
            return true
        end,
    },
    {
        name = "사자후술사",
        response = resp.message,
        cast_type = "INPUT",
        message = "술사 사자후 테스트 메시지입니다!",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 1000)
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.type == "SHOUT" then return true end
            return nil
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                error(string.format("사자후술사 MP failed: expected %d got %d", state.expected_mp, caster:mp()))
            end
            return true
        end,
    },
    {
        name = "사자후도적",
        response = resp.message,
        cast_type = "INPUT",
        message = "도적 사자후 테스트 메시지입니다!",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 1000)
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.type == "SHOUT" then return true end
            return nil
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                error(string.format("사자후도적 MP failed: expected %d got %d", state.expected_mp, caster:mp()))
            end
            return true
        end,
    },
    {
        name = "세계후",
        response = resp.message,
        cast_type = "INPUT",
        message = "세계후 테스트 메시지입니다!",
        pre = function(caster, _, state)
            caster:set_current_hp_mp(10000, 1000)
            state.expected_mp = caster:mp()
        end,
        condition = function(packet)
            if packet.type == "WORLD" then return true end
            return nil
        end,
        post = function(caster, _, state)
            if caster:mp() ~= state.expected_mp then
                error(string.format("세계후 MP failed: expected %d got %d", state.expected_mp, caster:mp()))
            end
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "SHOUT SPELL TEST STARTED")
    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end
    log("debug", "SHOUT SPELL TEST COMPLETED")
    return true
end
return M
