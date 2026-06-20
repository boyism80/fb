local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local MULTI_TARGET_SPELLS = {
    {
        name = "어검술",
        type = "NORMAL",
        spawn_positions = {{0, 1}, {-1, 2}, {0, 2}, {1, 2}},
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp * 2 / 3)
            return current_hp - hp_cost, current_mp - 40, nil
        end,
    },
    {
        name = "극'백호참",
        type = "NORMAL",
        spawn_positions = {{0, 1}, {-1, 2}, {0, 2}, {1, 2}, {-2, 3}, {-1, 3}, {0, 3}, {1, 3}, {2, 3}},
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 50, nil
        end,
    },
    {
        name = "파천검무",
        type = "NORMAL",
        spawn_positions = {{0, 1}, {0, 2}, {0, 3}, {0, 4}},
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            local pos = bot:position()
            return current_hp - hp_cost, current_mp - 180, {pos[1], pos[2] + 5}
        end,
    },
    {
        name = "초혼비무",
        type = "NORMAL",
        spawn_positions = {{0, 4}},
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp * 2 / 3)
            return current_hp - hp_cost, current_mp - 140, nil
        end,
    },
    {
        name = "쇄혼비무",
        type = "NORMAL",
        spawn_positions = {{0, 4}},
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp * 2 / 3)
            local pos = bot:position()
            return current_hp - hp_cost, current_mp - 140, {pos[1], pos[2] + 3}
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "MULTI-TARGET ATTACK_CAST SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(MULTI_TARGET_SPELLS))

    local spell_slot = 0
    for _, spell in ipairs(MULTI_TARGET_SPELLS) do
        local caster_pos = caster:position()

        caster:spawn_monsters_relative("다람쥐", spell.spawn_positions)
        caster:set_current_hp_mp(1000, 1000)

        local expected_hp, expected_mp, expected_position = spell.calculator(caster)

        caster:request(
            resp.update_internal,
            protocol.spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
            function(packet)
                local ok = true
                if expected_hp ~= nil then
                    ok = packet.ch_hp == expected_hp
                end
                if expected_mp ~= nil then
                    ok = ok and packet.ch_mp == expected_mp
                end
                if expected_position ~= nil then
                    ok = ok and skill.positions_equal(caster:position(), expected_position)
                end
                return ok
            end)

        spell_slot = spell_slot + 1
        caster:move_back(caster_pos, skill.DEFAULT_INTERVAL)
    end

    log("debug", "MULTI-TARGET ATTACK_CAST SPELL TEST COMPLETED")
    return true
end

return M
