local spell_runner = require("integration.lib.spell_runner")
local group = require("integration.lib.group")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local function build_cases(ctx)
    local function verify_group_healing_effects(before_hp_values, expected_hp_gain)
        for i = 1, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            local max_hp = bot:base_hp()
            local recoverable_amount = max_hp - before_hp_values[i + 1]
            local actual_hp_gain = math.min(expected_hp_gain, recoverable_amount)
            local expected_hp = before_hp_values[i + 1] + actual_hp_gain
            if bot:hp() ~= expected_hp then
                return false
            end
        end
        return true
    end

    return {
        {
            name = "백호의희원'첨",
            response = resp.update_internal,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                caster:set_current_hp_mp(10000, 10000)
                local current_mp = caster:mp()
                state.expected_hp_gain = current_mp * 2
                state.expected_mp_cost = math.floor(current_mp / 2)

                for i = 1, ctx:bot_count() - 1 do
                    local bot = ctx:bot(i)
                    bot:base_hp(100000)
                    bot:set_current_hp_mp(50, bot:mp())
                end

                state.before_hp_values = {}
                for i = 0, ctx:bot_count() - 1 do
                    table.insert(state.before_hp_values, ctx:bot(i):hp())
                end

                state.before_caster_mp = caster:mp()
                state.expected_mp = state.before_caster_mp - state.expected_mp_cost
            end,
            condition = function(packet, _, _, state)
                if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                    return nil
                end
                return true
            end,
            post = function(caster, _, state)
                ctx:sleep(500)
                if caster:mp() ~= state.expected_mp then
                    return false
                end
                return verify_group_healing_effects(state.before_hp_values, state.expected_hp_gain)
            end,
        },
        {
            name = "신령의기원'첨",
            response = resp.update_internal,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                caster:set_current_hp_mp(10000, 10000)
                state.expected_hp_gain = 10000
                state.expected_mp_cost = 1000

                for i = 1, ctx:bot_count() - 1 do
                    local bot = ctx:bot(i)
                    bot:base_hp(100000)
                    bot:set_current_hp_mp(50, bot:mp())
                end

                state.before_hp_values = {}
                for i = 0, ctx:bot_count() - 1 do
                    table.insert(state.before_hp_values, ctx:bot(i):hp())
                end

                state.before_caster_mp = caster:mp()
                state.expected_mp = state.before_caster_mp - state.expected_mp_cost
            end,
            condition = function(packet, _, _, state)
                if state.expected_mp ~= nil and packet.ch_mp ~= state.expected_mp then
                    return nil
                end
                return true
            end,
            post = function(caster, _, state)
                ctx:sleep(500)
                if caster:mp() ~= state.expected_mp then
                    return false
                end
                return verify_group_healing_effects(state.before_hp_values, state.expected_hp_gain)
            end,
        },
    }
end

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "Starting group healing spell test")
    local cases = build_cases(ctx)
    group.form(ctx)
    local success = spell_runner.run_cases(cases, caster, nil)
    group.cleanup(ctx)
    log("debug", "All group healing spell tests completed successfully!")
    return success
end

return M
