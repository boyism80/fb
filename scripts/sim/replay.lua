local facts  = require("sim.facts")
local battle = require("brain.policies.battle")

local M = {}

local function median(values)
    if #values == 0 then
        return nil
    end
    table.sort(values)
    return values[(#values + 1) // 2]
end

local function find(snap, oid)
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.oid == oid then
            return mob
        end
    end
    return nil
end

local function casts(uses)
    local total = 0
    for key, count in pairs(uses or {}) do
        if key ~= "SWING" and string.sub(key, 1, 4) ~= "ACT:" then
            total = total + count
        end
    end
    return total
end

local function band(level)
    return ((level or 1) - 1) // 10 * 10 + 1
end

function M.run(options)
    local data    = require(options.data)
    local mind    = brain.new()
    local groups  = {}
    local missing = 0
    for _, record in ipairs(data.records) do
        local snap           = record.snap
        snap.record          = snap.record or { failures = {}, memory = {} }
        snap.record.failures = snap.record.failures or {}
        snap.record.memory   = snap.record.memory or {}
        snap.world.mobs      = snap.world.mobs or {}
        snap.world.items     = snap.world.items or {}
        snap.bag             = snap.bag or { items = {}, used = 0, capacity = 0 }
        snap.pace            = snap.pace or {}
        local mob            = find(snap, record.oid)
        if mob == nil or record.result ~= "KILLED" then
            missing = missing + 1
        else
            local bound  = facts.bind(mind, snap)
            local choice = battle.choose(snap, bound, mob)
            if choice ~= nil then
                local now   = battle.forecast(snap, bound, mob, choice)
                local key   = string.format("%s|%d|%s|%s", tostring(record.class), band(record.level),
                    tostring(record.mob),
                    tostring(choice.key))
                local group = groups[key] or
                    { actual = {}, before = {}, after = {}, casts = {}, model = {}, taken = {}, predicted = {} }
                groups[key] = group
                table.insert(group.actual, record.actual.time)
                table.insert(group.before, record.forecast.fight and record.forecast.fight.time or -1)
                table.insert(group.after, now.fight.time)
                table.insert(group.casts, casts(record.actual.uses))
                table.insert(group.model, now.model.casts)
                table.insert(group.taken, math.max(0, -record.actual.hp))
                table.insert(group.predicted, math.max(0, now.fight.taken - now.fight.healed))
            end
        end
    end
    local keys = {}
    for key in pairs(groups) do
        table.insert(keys, key)
    end
    table.sort(keys)
    for _, key in ipairs(keys) do
        local group = groups[key]
        local actual, before, after = median(group.actual), median(group.before), median(group.after)
        log("debug", string.format(
            "[sim:replay] %s count=%d time=%.0f before=%.0f after=%.0f ratio_before=%.2f ratio_after=%.2f casts=%s model_casts=%s taken=%.0f predicted=%.0f",
            key, #group.actual, actual, before, after, before > 0 and actual / before or -1,
            after > 0 and actual / after or -1, tostring(median(group.casts)), tostring(median(group.model)),
            median(group.taken), median(group.predicted)))
    end
    log("debug", string.format("[sim:replay] records=%d groups=%d skipped=%d", #data.records, #keys, missing))
    return #keys > 0
end

return M
