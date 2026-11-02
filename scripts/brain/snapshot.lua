local states = require("brain.data.states")

local M = {}

local function failures(brain, bot)
    local result = {}
    for _, state in ipairs(states) do
        for _, task in ipairs(state.tasks) do
            local record = brain:failure(bot, task)
            if record ~= nil then
                result[task] = record
            end
        end
    end
    return result
end

function M.take(brain, bot)
    local seen     = brain:observe(bot)
    local position = bot:position()
    return {
        self    = {
            name      = bot:name(),
            level     = seen.level,
            exp       = seen.exp,
            class     = seen.class,
            promotion = seen.promotion,
            nation    = seen.nation,
            ghost     = seen.ghost,
            online    = seen.online,
            hp        = seen.hp,
            max_hp    = seen.max_hp,
            mp        = seen.mp,
            max_mp    = seen.max_mp,
            money     = seen.money,
            phydef    = seen.phydef,
            dam       = seen.dam,
            hit       = seen.hit,
            struck    = seen.struck,
        },
        buffs   = seen.buffs,
        spells  = seen.spells,
        gear    = brain:worn(bot),
        bag     = { items = brain:inventory(bot), used = seen.bag, capacity = seen.capacity },
        world   = {
            map    = seen.map,
            x      = position[1],
            y      = position[2],
            mobs   = brain:mobs(bot),
            items  = brain:items(bot),
            dialog = seen.dialog,
            aside  = brain:aside(bot),
        },
        record  = {
            current  = brain:state(bot),
            failures = failures(brain, bot),
            death    = brain:death(bot),
            jumped   = seen.jumped,
            memory   = brain:recall(bot),
        },
        pace    = seen.pace,
        now     = seen.now,
        persona = brain:persona(bot),
    }
end

return M
