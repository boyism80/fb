local teleport = require("brain.data.teleport")

local M = {}

local REQUEST_COST = 1

function M.jumps(brain, bot)
    local seen  = brain:observe(bot)
    local jumps = {}
    for _, spell in ipairs(teleport.spells) do
        local ready = bot:spell_slot(spell.name) ~= 0xFF and (spell.ghost == true) == seen.ghost and
            bot:mp() >= (spell.mp or 0)
        if ready then
            table.insert(jumps, {
                spell  = spell.name,
                via    = spell.via,
                to     = spell.returns and teleport.returns[seen.nation] or spell.to,
                inputs = spell.inputs,
                cost   = REQUEST_COST,
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
                    cost = REQUEST_COST,
                })
            end
        end
    end
    return jumps
end

return M
