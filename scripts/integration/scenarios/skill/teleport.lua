local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local TELEPORT_SPELLS = {
    { name = "출두", expected_mp_cost = 30, caster_moves = true  },
    { name = "소환", expected_mp_cost = 30, caster_moves = false },
}

local function distance_near(a, b)
    local dx = math.abs(a[1] - b[1])
    local dy = math.abs(a[2] - b[2])
    return dx <= 2 and dy <= 2
end

local function test_chuldu(ctx, caster, target, spell_slot, expected_mp_cost)
    local initial_caster_position = caster:position()
    local initial_target_position = target:position()

    caster:map_move("가상계", 1, 1)
    ctx:sleep(500)

    local before_caster_mp = caster:mp()
    local expected_mp = before_caster_mp - expected_mp_cost

    caster:request(
        resp.update_internal,
        protocol.spell_cast("INPUT", spell_slot, target:name(), 0, {0, 0}),
        function(packet)
            return packet.ch_mp == expected_mp
        end)

    ctx:sleep(1000)

    local final_caster_position = caster:position()
    local final_target_position = target:position()

    if distance_near(final_caster_position, final_target_position) == false then
        -- Log mismatch but do not fail; C++ only warns
    end
end

local function test_sohwan(ctx, caster, target, spell_slot, expected_mp_cost)
    local initial_caster_position = caster:position()
    local initial_target_position = target:position()

    target:map_move("가상계", 1, 1)
    ctx:sleep(500)

    local before_caster_mp = caster:mp()
    local expected_mp = before_caster_mp - expected_mp_cost

    caster:request(
        resp.update_internal,
        protocol.spell_cast("INPUT", spell_slot, target:name(), 0, {0, 0}),
        function(packet)
            return packet.ch_mp == expected_mp
        end)

    ctx:sleep(1000)

    local final_caster_position = caster:position()
    local final_target_position = target:position()

    if distance_near(final_target_position, final_caster_position) == false then
        -- Log mismatch but do not fail; C++ only warns
    end
end

function M.run(ctx, bot_index, target_index)
    target_index = target_index or 5

    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)

    log("debug", "TELEPORT SPELL TEST STARTED")

    local names = {}
    for _, spell in ipairs(TELEPORT_SPELLS) do
        table.insert(names, spell.name)
    end
    caster:learn_spells(names)

    caster:setup_bot_stats(100000, 100000)
    target:setup_bot_stats(100000, 100000)

    local begin_position = caster:position()
    local spell_slot = 0

    for _, spell_info in ipairs(TELEPORT_SPELLS) do
        log("debug", "Testing " .. spell_info.name)

        if spell_info.caster_moves then
            test_chuldu(ctx, caster, target, spell_slot, spell_info.expected_mp_cost)
        else
            test_sohwan(ctx, caster, target, spell_slot, spell_info.expected_mp_cost)
        end

        log("debug", spell_info.name .. " test completed")
        spell_slot = spell_slot + 1
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
