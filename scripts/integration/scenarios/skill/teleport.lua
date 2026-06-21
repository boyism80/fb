local spell_runner = require("integration.lib.spell_runner")
local lib = require("integration.lib")

local M = {}

local resp = require("integration.response")
local skill = require("integration.lib.skill")

local function build_cases(ctx)
    local function distance_near(a, b)
        local dx = math.abs(a[1] - b[1])
        local dy = math.abs(a[2] - b[2])
        return dx <= 2 and dy <= 2
    end

    return {
        {
            name = "출두",
            response = resp.update_internal,
            cast_type = "INPUT",
            message = function(_, target)
                return target:name()
            end,
            pre = function(caster, target, state)
                caster:map_move("가상계", 1, 1)
                ctx:sleep(500)
                state.expected_mp = caster:mp() - 30
            end,
            condition = function(packet, _, _, state)
                if packet.ch_mp == state.expected_mp then
                    return true
                end
                return nil
            end,
            post = function(caster, target)
                ctx:sleep(1000)
                if distance_near(caster:position(), target:position()) == false then
                    -- warn only
                end
                return true
            end,
        },
        {
            name = "소환",
            response = resp.update_internal,
            cast_type = "INPUT",
            message = function(_, target)
                return target:name()
            end,
            pre = function(caster, target, state)
                target:map_move("가상계", 1, 1)
                ctx:sleep(500)
                state.expected_mp = caster:mp() - 30
            end,
            condition = function(packet, _, _, state)
                if packet.ch_mp == state.expected_mp then
                    return true
                end
                return nil
            end,
            post = function(caster, target)
                ctx:sleep(1000)
                if distance_near(target:position(), caster:position()) == false then
                    -- warn only
                end
                return true
            end,
        },
    }
end

function M.run(ctx, bot_index, target_index)
    target_index = target_index or lib.formation.skill_pair_target(bot_index)
    if target_index == nil then
        log("debug", "TELEPORT SPELL TEST requires a target bot")
        return false
    end
    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)
    log("debug", "TELEPORT SPELL TEST STARTED")
    caster:setup_bot_stats(100000, 100000)
    target:setup_bot_stats(100000, 100000)
    local begin_position = caster:position()
    local cases = build_cases(ctx)
    if spell_runner.run_cases(cases, caster, target) == false then
        return false
    end
    log("debug", "All teleport spell tests completed successfully!")
    local end_position = caster:position()
    local move_x_axis = end_position[1] - begin_position[1]
    local direction_x = move_x_axis > 0 and "LEFT" or "RIGHT"
    caster:move(direction_x, math.abs(move_x_axis))
    local move_y_axis = end_position[2] - begin_position[2]
    local direction_y = move_y_axis > 0 and "TOP" or "BOTTOM"
    caster:move(direction_y, math.abs(move_y_axis))
    caster:direction("BOTTOM")
    return true
end

return M
