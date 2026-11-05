local combat   = require("brain.data.combat")
local teleport = require("brain.data.teleport")

local M = {}

function M.jumps(brain, bot, urgent)
    local seen  = brain:observe(bot)
    local jumps = {}
    local cost  = (urgent or seen.ghost) and 1 or math.ceil(combat.transfer / ((seen.pace or {}).step or combat.stride))
    for _, spell in ipairs(teleport.spells) do
        local ready = bot:spell_slot(spell.name) ~= 0xFF and (spell.ghost == true) == seen.ghost and
            bot:mp() >= (spell.mp or 0)
        if ready then
            table.insert(jumps, {
                spell  = spell.name,
                via    = spell.via,
                to     = spell.returns and teleport.returns[seen.nation] or spell.to,
                inputs = spell.inputs,
                cost   = cost,
            })
        end
    end

    if seen.ghost == false then
        for _, item in ipairs(teleport.items) do
            if bot:item_count(item.name) > 0 then
                table.insert(jumps, {
                    item = item.name,
                    via  = item.via,
                    to   = item.returns and teleport.returns[seen.nation] or item.to,
                    cost = cost,
                })
            end
        end
    end
    return jumps
end

return M
