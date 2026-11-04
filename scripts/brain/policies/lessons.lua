local learning = require("brain.data.learning")
local outfits  = require("brain.data.outfits")
local shops    = require("brain.data.shops")

local M = {}

local function holding(snap)
    local counts = {}
    for _, item in ipairs(snap.bag.items) do
        counts[item.name] = (counts[item.name] or 0) + item.count
    end
    return counts
end

function M.missing(snap, entry)
    local counts = holding(snap)
    local result = { bought = {}, dropped = {}, price = 0, drops = 0 }
    for name, count in pairs(entry.materials or {}) do
        local short = count - (counts[name] or 0)
        if short > 0 then
            local source = shops[name] or outfits[name]
            if source ~= nil then
                result.bought[name] = short
                result.price        = result.price + short * source.price
            else
                result.dropped[name] = short
                result.drops         = result.drops + short
            end
        end
    end
    return result
end

function M.open(snap)
    local known = {}
    for _, learned in ipairs(snap.spells or {}) do
        known[learned.name] = true
    end
    local found = {}
    for _, entry in ipairs(learning[snap.self.class] or {}) do
        if entry.level <= (snap.self.level or 0) then
            for _, name in ipairs(entry.names) do
                if known[name] == nil then
                    table.insert(found, { entry = entry, name = name })
                end
            end
        end
    end
    return found
end

function M.taught(snap, name, entry)
    local spells = {}
    for _, learned in ipairs(snap.spells or {}) do
        table.insert(spells, learned)
    end
    table.insert(spells, { name = name, slot = 0 })
    local owed = {}
    for material, count in pairs(entry.materials or {}) do
        owed[material] = count
    end
    local items = {}
    for _, item in ipairs(snap.bag.items) do
        local paid = math.min(item.count, owed[item.name] or 0)
        owed[item.name] = (owed[item.name] or 0) - paid
        if item.count > paid then
            table.insert(items, setmetatable({ count = item.count - paid }, { __index = item }))
        end
    end
    local bag = setmetatable({ items = items, used = #items }, { __index = snap.bag })
    return setmetatable({ spells = spells, bag = bag, hypothesis = name, actual = snap }, { __index = snap })
end

return M
