local job      = require("brain.data.job")
local learning = require("brain.data.learning")
local reserved = require("brain.data.reserved")

local M = {}

function M.unpack(item)
    local name, uses = item.name:match("^(.-)%s*%[(%d+)%]")
    if name == nil then
        return item.name, item.count
    end
    return name, item.count * tonumber(uses)
end

function M.keep(snap)
    local keep = {}
    for name in pairs(reserved) do
        keep[name] = math.huge
    end
    if snap.self.class == "NONE" then
        keep[job.offering.name] = math.max(keep[job.offering.name] or 0, job.offering.count)
    end
    local learned = {}
    for _, spell in ipairs(snap.spells or {}) do
        learned[spell.name] = true
    end
    for _, entry in ipairs(learning[snap.self.class] or {}) do
        local known = false
        for _, name in ipairs(entry.names) do
            known = known or learned[name] == true
        end
        if known == false and entry.level <= snap.self.level then
            for name, count in pairs(entry.materials) do
                keep[name] = (keep[name] or 0) + count
            end
        end
    end
    return keep
end

function M.room(snap, name, capacity, freed)
    local free = math.max(0, snap.bag.capacity - snap.bag.used) + (freed or 0)
    local room = free * capacity
    for _, item in ipairs(snap.bag.items) do
        if item.name == name then
            room = room + math.max(0, capacity - item.count)
        end
    end
    return room
end

function M.usable(snap)
    local keep  = M.keep(snap)
    local found = {}
    for _, item in ipairs(snap.bag.items) do
        local name, count = M.unpack(item)
        found[name] = (found[name] or 0) + count
    end
    for name, count in pairs(found) do
        found[name] = math.max(0, count - (keep[name] or 0))
    end
    return found
end

return M
