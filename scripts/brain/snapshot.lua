local combat = require("brain.data.combat")
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

local function grouped(text)
    local names  = {}
    local leader = nil
    local first  = true
    for line in string.gmatch(text or "", "[^\n]+") do
        local name = first == false and string.match(line, "^%s+%*?%s*(%S+)%s*$") or nil
        if name ~= nil then
            table.insert(names, name)
            if string.find(line, "*", 1, true) ~= nil then
                leader = name
            end
        end
        first = false
    end
    return names, leader
end

local function party(brain, bot, names)
    local wanted = {}
    for _, name in ipairs(names) do
        wanted[name] = true
    end
    local found = {}
    for _, entry in ipairs(brain:roster(bot)) do
        if wanted[entry.name] and type(entry.profile) == "table" then
            table.insert(found, entry.profile)
        end
    end
    return found
end

function M.take(brain, bot)
    local seen           = brain:observe(bot)
    local position       = bot:position()
    local names          = {}
    local listed, leader = grouped(seen.group)
    for _, name in ipairs(listed) do
        if name ~= bot:name() then
            table.insert(names, name)
        end
    end
    return {
        self    = {
            name      = bot:name(),
            level     = seen.level,
            exp       = seen.exp,
            class     = seen.class,
            promotion = seen.promotion,
            gender    = bot:gender(),
            str       = bot:str(),
            dex       = bot:dex(),
            int       = bot:int(),
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
            casted    = seen.casted,
            grouping  = seen.grouping,
            cloaked   = seen.cloaked,
            riding    = seen.riding,
        },
        buffs   = seen.buffs,
        spells  = seen.spells,
        gear    = brain:worn(bot),
        bag     = { items = brain:inventory(bot), used = seen.bag, capacity = seen.capacity },
        world   = {
            map      = seen.map,
            x        = position[1],
            y        = position[2],
            mobs     = brain:mobs(bot),
            items    = brain:items(bot),
            dialog   = seen.dialog,
            no_spell = seen.no_spell,
            aside    = brain:aside(bot),
        },
        record  = {
            current  = brain:state(bot),
            failures = failures(brain, bot),
            death    = brain:death(bot),
            jumped   = seen.jumped,
            memory   = brain:recall(bot),
        },
        pace    = setmetatable({ step = combat.walk }, { __index = seen.pace }),
        now     = seen.now,
        persona = brain:persona(bot),
        group   = names,
        leader  = leader,
        party   = party(brain, bot, names),
    }
end

return M
