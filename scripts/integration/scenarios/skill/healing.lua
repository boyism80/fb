local resp = require("integration.response")
local spell_runner = require("integration.lib.spell_runner")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local CASES = {
    {
        name = "누리의기원",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 50
            state.expected_mp = nil
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            return true
        end,
    },
    {
        name = "하늘의기원",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 200
            state.expected_mp = nil
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            return true
        end,
    },
    {
        name = "대지의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 50
            state.expected_mp = caster:mp() - 50
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "동해의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 100
            state.expected_mp = caster:mp() - 30
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "바다의기원",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 100
            state.expected_mp = nil
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            return true
        end,
    },
    {
        name = "천공의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 200
            state.expected_mp = caster:mp() - 100
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "구름의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 500
            state.expected_mp = caster:mp() - 120
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "태양의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 1000
            state.expected_mp = caster:mp() - 240
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "생명의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 5000
            state.expected_mp = caster:mp() - 300
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "현자의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 5000
            state.expected_mp = caster:mp() - 1000
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "신령의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 10000
            state.expected_mp = caster:mp() - 1000
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "봉황의기원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 30000
            state.expected_mp = caster:mp() - 10000
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "천공의희원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 200
            state.expected_mp = caster:mp() - 200
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "구름의희원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 500
            state.expected_mp = caster:mp() - 480
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
    {
        name = "태양의희원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            state.expected_hp = caster:hp() + 1000
            state.expected_mp = caster:mp() - 960
        end,
        condition = function(packet, caster, target, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
}

local resp = require("integration.response")

local BAEKHO_CASE = {
    {
        name = "백호의희원",
        response = resp.update_internal,
        cast_type = "TARGET",
        oid = function(caster) return caster:oid() end,
        position = function(caster) return caster:position() end,
        pre = function(caster, _, state)
            caster:mp(100)
            caster:hp(50)

            local current_mp = caster:mp()
            local theoretical_hp_gain = current_mp * 2
            local expected_mp_cost = math.floor(current_mp / 2)
            local caster_max_hp = caster:base_hp()
            local caster_current_hp = caster:hp()
            local caster_recoverable_hp = caster_max_hp - caster_current_hp
            local actual_expected_hp_gain = math.min(theoretical_hp_gain, caster_recoverable_hp)

            if theoretical_hp_gain > caster_recoverable_hp then
                local new_caster_hp = caster_max_hp - theoretical_hp_gain
                if new_caster_hp < 1 then
                    new_caster_hp = 1
                end
                caster:hp(new_caster_hp)
                actual_expected_hp_gain = theoretical_hp_gain
            end

            state.expected_hp = caster:hp() + actual_expected_hp_gain
            state.expected_mp = caster:mp() - expected_mp_cost
        end,
        condition = function(packet, _, _, state)
            if state.expected_hp ~= nil and packet.ch_hp ~= state.expected_hp then
                return nil
            end
            if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "HEALING SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000, 50)
    caster:base_hp(100000)
    caster:hp(50)
    if spell_runner.run_cases(CASES, caster, nil) == false then
        return false
    end
    local slot = caster:learn_spell("백호의희원")
    if slot == 0xFF then
        return false
    end
    if spell_runner.run_case(BAEKHO_CASE[1], caster, nil, slot) == false then
        return false
    end
    log("debug", "All healing spell tests completed successfully!")
    return true
end

return M
