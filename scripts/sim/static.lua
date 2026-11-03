local facts     = require("sim.facts")
local serialize = require("sim.serialize")
local hunting   = require("brain.data.hunting")
local combat    = require("brain.policies.combat")
local escort    = require("brain.policies.escort")
local ground    = require("brain.policies.ground")
local score     = require("brain.policies.score")
local survival  = require("brain.policies.survival")

local M = {}

local TEAMS = {
    { "WARRIOR", "POET" },
    { "ROGUE", "POET" },
    { "MAGE", "POET" },
    { "WARRIOR", "ROGUE", "POET" },
}

local function median(values)
    if #values == 0 then
        return 0
    end
    table.sort(values)
    return values[(#values + 1) // 2]
end

local function knows(snap, name)
    for _, spell in ipairs(snap.spells or {}) do
        if spell.name == name then
            return true
        end
    end
    return false
end

local function cells(records, band, per, requires)
    local found = {}
    for _, record in ipairs(records) do
        local snap = record.snap
        local need = (requires or {})[snap.self.class]
        if snap.self.class ~= "NONE" and (need == nil or knows(snap, need)) then
            local key = snap.self.class .. "|" .. ((snap.self.level - 1) // band)
            found[key] = found[key] or {}
            if #found[key] < per then
                table.insert(found[key], snap)
            end
        end
    end
    return found
end

local function solo(mind, snap)
    local bound = facts.bind(mind, snap)
    local best  = ground.choose(snap, bound, ground.mode(snap))
    return best, bound
end

local function damage(floor, result)
    local wanted = {}
    for _, name in ipairs(result.targets) do
        wanted[name] = true
    end
    local hp    = 0
    local count = 0
    for _, entry in ipairs(floor.mobs) do
        local mob = hunting.mobs[entry.id]
        if mob ~= nil and wanted[mob.name] then
            hp    = hp + entry.count * (mob.hp or 0)
            count = count + entry.count
        end
    end
    return count > 0 and result.kills * hp / count or 0
end

local floormap = nil

local function lookup(map)
    if floormap == nil then
        floormap = {}
        for _, bundle in ipairs(hunting.bundles) do
            for _, floor in ipairs(bundle.floors) do
                floormap[floor.map] = floor
            end
        end
    end
    return floormap[map]
end

local function explain(snap, query, crew, best)
    local rested  = score.rested(snap, "REFILL")
    local ability = score.capability(rested, query)
    local floor   = lookup(best.floors[#best.floors].map)
    for _, entry in ipairs(floor.mobs) do
        local mob = hunting.mobs[entry.id]
        if mob ~= nil then
            local context = escort.context(rested, query, mob, crew, floor.density, ability.refill.hp)
            local result  = survival.fight(rested, query, mob, context)
            local own     = combat.outcome(rested, query, mob, floor.density, context.aid)
            log("debug", string.format(
                "[sim:why] %s lv=%d hp=%d mp=%d %s mob=%s hp=%d count=%d share=%.2f aid=%.2f fight=%.0fms own=%.0fms casts=%d taken=%.0f healed=%.0f ending=%.0f",
                snap.self.class, snap.self.level, rested.self.max_hp, rested.self.max_mp or 0, best.name, mob.name,
                mob.hp or 0, entry.count, context.share or -1, context.aid or 0, result.time, own.time, own.casts or 0,
                result.taken, result.healed, result.ending))
        end
    end
end

local function sustained(snap, query, crew, trace)
    local mode = crew ~= nil and "REFILL" or ground.mode(snap)
    local best = ground.choose(snap, query, mode, crew)
    if best == nil then
        return 0, "nil"
    end
    if trace then
        explain(snap, query, crew, best)
    end
    local total = 0
    for _, result in ipairs(best.floors) do
        local floor = lookup(result.map)
        if floor ~= nil then
            total = total + damage(floor, result)
        end
    end
    return total / #best.floors, best.name
end

function M.dps(options)
    local data = require(options.data)
    local band = options.band or data.config.band
    local mind = brain.new()
    local grid = cells(data.records, band, options.per, options.requires)
    local rows = {}
    for key, snaps in pairs(grid) do
        local class, slot = key:match("^(%w+)|(%d+)$")
        local poets       = grid["POET|" .. slot] or {}
        local solos       = {}
        local teams       = {}
        for index, snap in ipairs(snaps) do
            local bound = facts.bind(mind, snap)
            table.insert(solos, (sustained(snap, bound, nil)))
            local poet = poets[index]
            if poet ~= nil and poet ~= snap then
                table.insert(teams, (sustained(snap, bound, escort.crew({ score.rested(poet, "REFILL") }, bound),
                    options.trace == class and index == 1)))
            end
        end
        table.insert(rows, {
            class = class,
            band = tonumber(slot) * band + 1,
            solo = median(solos),
            team = median(teams),
            count = #snaps
        })
    end
    table.sort(rows, function(left, right)
        return left.band < right.band or (left.band == right.band and left.class < right.class)
    end)
    for _, row in ipairs(rows) do
        log("debug", string.format("[sim:dps] %d %s solo=%.0f with_poet=%.0f count=%d", row.band, row.class, row.solo,
            row.team, row.count))
    end
    return #rows > 0
end

function M.run(options)
    local data    = require(options.data)
    local band    = options.band or data.config.band
    local mind    = brain.new()
    local started = os.clock()
    local grid    = cells(data.records, band, options.per)
    local rows    = {}
    local solos   = {}
    for key, snaps in pairs(grid) do
        local rates  = {}
        local deaths = {}
        local empty  = 0
        solos[key]   = {}
        for index, snap in ipairs(snaps) do
            local best = solo(mind, snap)
            solos[key][index] = best ~= nil and best.rate or 0
            if options.trace then
                local floor = best ~= nil and best.floors[#best.floors] or nil
                log("debug", string.format(
                    "[sim:trace] %s lv=%d hp=%d mp=%d phydef=%d money=%d map=%d spells=%d best=%s floors=%d rate=%.1f margin=%.2f deaths=%.4f targets=%s",
                    snap.self.class, snap.self.level, snap.self.max_hp, snap.self.max_mp, snap.self.phydef,
                    snap.self.money, snap.world.map, #snap.spells, best ~= nil and best.name or "nil",
                    best ~= nil and #best.floors or 0, best ~= nil and best.rate or 0, best ~= nil and best.margin or 0,
                    best ~= nil and best.deaths * 3600 or 0,
                    floor ~= nil and table.concat(floor.targets or {}, ",") or ""))
                local bound = facts.bind(mind, snap)
                local top   = {}
                for rank, entry in ipairs(ground.ranked(snap, bound, ground.mode(snap))) do
                    if rank > 3 then
                        break
                    end
                    table.insert(top, string.format("%s:%.1f:%s", entry.candidate.bundle.name, entry.rate,
                        tostring(ground.reachable(snap, bound, entry.candidate))))
                end
                log("debug", string.format("[sim:top] %s lv=%d map=%d mode=%s %s", snap.self.class, snap.self.level,
                    snap.world.map, tostring(ground.mode(snap)), table.concat(top, " ")))
            end
            if best == nil then
                empty = empty + 1
            else
                table.insert(rates, best.rate)
                table.insert(deaths, best.deaths * 3600)
            end
        end
        local class, slot = key:match("^(%w+)|(%d+)$")
        table.insert(rows, {
            class = class,
            band = tonumber(slot) * band + 1,
            count = #snaps,
            empty = empty,
            rate = median(rates),
            deaths = median(deaths),
        })
    end
    local teams = {}
    for _, team in ipairs(TEAMS) do
        for slot = 0, 99 // band do
            local lists = {}
            local shortest = math.huge
            for _, class in ipairs(team) do
                local list = grid[class .. "|" .. slot] or {}
                table.insert(lists, list)
                shortest = math.min(shortest, #list)
            end
            if shortest > 0 and shortest < math.huge then
                local rates   = {}
                local winners = 0
                for index = 1, math.min(shortest, options.teams) do
                    local members = {}
                    for position in ipairs(team) do
                        table.insert(members, lists[position][index])
                    end
                    local plan = ground.party(members, facts.bind(mind, members[1]), "SUPPLY")
                    local rate = plan ~= nil and plan.rate or 0
                    table.insert(rates, rate)
                    local all = true
                    for position, class in ipairs(team) do
                        all = all and rate > (solos[class .. "|" .. slot][index] or 0)
                    end
                    winners = winners + (all and 1 or 0)
                end
                table.insert(teams, {
                    team = table.concat(team, "+"),
                    band = slot * band + 1,
                    count = #rates,
                    rate = median(rates),
                    winners = winners,
                })
            end
        end
    end
    table.sort(rows, function(left, right)
        return left.class < right.class or (left.class == right.class and left.band < right.band)
    end)
    table.sort(teams, function(left, right)
        return left.team < right.team or (left.team == right.team and left.band < right.band)
    end)
    local elapsed = os.clock() - started
    for _, row in ipairs(rows) do
        log("debug", string.format("[sim:solo] %s %d count=%d empty=%d rate=%.1f deaths=%.4f/h", row.class, row.band,
            row.count, row.empty, row.rate, row.deaths))
    end
    for _, row in ipairs(teams) do
        log("debug", string.format("[sim:team] %s %d count=%d rate=%.1f winners=%d", row.team, row.band, row.count,
            row.rate, row.winners))
    end
    log("debug", string.format("[sim:static] solo=%d teams=%d elapsed=%.1fs", #rows, #teams, elapsed))
    if options.output ~= nil then
        serialize.write(options.output, { config = data.config, solo = rows, teams = teams, elapsed = elapsed })
    end
    return #rows > 0
end

return M
