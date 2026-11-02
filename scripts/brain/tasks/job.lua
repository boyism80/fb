local protocol = require("integration.protocol")
local career   = require("brain.policies.job")
local hunt     = require("brain.policies.hunt")
local travel   = require("brain.policies.travel")
local job      = require("brain.data.job")
local tutorial = require("brain.tasks.tutorial")

local M = {}

local function gather(brain, bot)
    local arrived, event = travel.go(brain, bot, { maps = hunt.beginner(brain, bot, job.offering.from) })
    if arrived == false then
        return travel.yielded(event), event and event.type
    end

    local wanted = {}
    for _, name in ipairs(job.offering.from) do
        wanted[name] = true
    end
    local result, reason = hunt.step(brain, bot, {
        key    = "offering",
        names  = job.offering.from,
        loot   = { job.offering.name },
        full   = function()
            local seen = brain:observe(bot)
            return seen.bag >= seen.capacity
        end,
        wants  = function(_, name)
            return wanted[name] == true
        end,
        killed = function()
        end,
        done   = function()
            return bot:item_count(job.offering.name) >= job.offering.count
        end,
    })
    return result ~= "FAILED", reason
end

local function enlist(brain, bot, chosen)
    local master = job.masters[chosen]
    local spots  = brain:npcs(bot, { master })
    local maps   = {}
    for _, spot in ipairs(spots) do
        table.insert(maps, spot.map)
    end
    if #maps == 0 then
        return false
    end
    local arrived, event = travel.go(brain, bot, { maps = maps })
    if arrived == false then
        return false, travel.yielded(event) and "ENGAGED" or nil
    end

    local spot = nil
    for _, candidate in ipairs(spots) do
        if candidate.map == bot:map() then
            spot = candidate
        end
    end
    if spot == nil then
        return false
    end
    local near, moved = travel.go(brain, bot, { map = spot.map, x = spot.x, y = spot.y, range = 1 })
    if near == false then
        return false, travel.yielded(moved) and "ENGAGED" or nil
    end

    local npc = brain:npc(bot, master)
    if npc == nil then
        return false
    end
    brain:talk(bot, protocol.click(npc.oid), job.oath)
    return brain:observe(bot).class == chosen
end

function M.wanted(snap)
    return snap.self.ghost == false and snap.self.class == "NONE" and snap.self.level >= job.level and
        tutorial.step(snap) == 0
end

function M.run(brain, bot)
    if brain:observe(bot).class ~= "NONE" then
        career.settle(brain, bot)
        return true
    end

    local chosen = career.choose(brain, bot)
    if chosen == nil then
        return false
    elseif bot:item_count(job.offering.name) < job.offering.count and brain:observe(bot).bag >= brain:observe(bot).capacity then
        return false, "FULL"
    elseif bot:item_count(job.offering.name) < job.offering.count then
        return gather(brain, bot)
    else
        local joined, reason = enlist(brain, bot, chosen)
        if joined then
            career.settle(brain, bot)
        end
        return joined or reason == "ENGAGED", reason
    end
end

return M
