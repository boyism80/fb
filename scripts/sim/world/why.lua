local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local ground   = require("brain.policies.ground")
local hunting  = require("brain.data.hunting")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local score    = require("brain.policies.score")
local survival = require("brain.policies.survival")

local M = {}


local function fight(snap, query, mob, crew)
    local context = escort.context(snap, query, mob, crew, nil, recovery.rates(snap).hp)
    local first   = survival.fight(snap, query, mob, context)
    local crowd   = mob.oid ~= nil and survival.contact(snap, mob, first.time) or nil
    context.crowd = crowd
    local result  = survival.fight(snap, query, mob, context)
    return {
        time = result.time < math.huge and result.time // 1 or -1,
        ending = result.ending > -math.huge and result.ending // 1 or -1,
        taken = result.taken < math.huge and result.taken // 1 or -1,
        healed = result.healed // 1,
        drained = (result.drained or 0) // 1,
        crowd = crowd ~= nil and crowd.extra or 0,
        pressure = crowd ~= nil and crowd.pressure or 0,
    }
end

local function floorview(plan, map)
    for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
        if floor.map == map then
            return {
                map = floor.map,
                rate = floor.rate,
                kills = floor.kills,
                crowd = floor.crowd,
                need = floor.need,
                margin = floor.margin,
                wear = floor.wear,
                pain = floor.pain,
                drain = floor.drain,
                deaths = floor.deaths,
                safe = floor.safe,
                targets = floor.targets,
            }
        end
    end
    return nil
end

function M.escape(snap, query, decision)
    local name = (decision.cause or ""):match("^%u+:(.*)$")
    local mob  = nil
    for _, seen in ipairs(snap.world.mobs or {}) do
        if seen.name == name then
            mob = seen
            break
        end
    end
    if mob == nil then
        return nil
    end
    local present = escort.present(snap, query)
    local cohort  = escort.gathering(snap, query)
    local whole   = {}
    for _, profile in ipairs(snap.party or {}) do
        table.insert(whole, escort.member(profile))
    end
    local members = {}
    for _, member in ipairs(party.members(snap)) do
        table.insert(members, {
            name = member.self.name,
            map = member.world.map,
            x = member.world.x,
            y = member.world.y,
            task = member.task,
            heading = member.heading,
            hp = member.self.hp,
            mp = member.self.mp,
        })
    end
    local adjacent = 0
    for _, seen in ipairs(snap.world.mobs or {}) do
        if seen.x ~= nil and math.abs(seen.x - snap.world.x) + math.abs(seen.y - snap.world.y) <= 1 then
            adjacent = adjacent + 1
        end
    end
    local plan   = party.plan(snap, query)
    local rested = score.rested(snap)
    return {
        hp = snap.self.hp,
        max_hp = snap.self.max_hp,
        mp = snap.self.mp,
        max_mp = snap.self.max_mp,
        phydef = snap.self.phydef,
        x = snap.world.x,
        y = snap.world.y,
        mob = {
            name = mob.name,
            id = mob.id,
            hp = mob.hp,
            damage = mob.damage,
            least = mob.least,
            speed = mob.speed,
            defense = mob.defense,
            x = mob.x,
            y = mob.y,
            attack = mob.attack
        },
        seen = #(snap.world.mobs or {}),
        adjacent = adjacent,
        burst = battle.burst(snap),
        blow = combat.blow(snap, mob),
        alone = fight(snap, query, mob, nil),
        present = present ~= nil and fight(snap, query, mob, present) or nil,
        cohort = cohort ~= nil and fight(snap, query, mob, cohort) or nil,
        whole = #whole > 0 and fight(snap, query, mob, escort.crew(whole, query)) or nil,
        rested_alone = fight(rested, query, mob, nil),
        members = members,
        plan = plan ~= nil and plan.name or nil,
        plan_rate = plan ~= nil and plan.rate or nil,
        floor = floorview(plan, snap.world.map),
        mode = recovery.reserve(snap),
    }
end

function M.plan(snap, query)
    local plan = party.plan(snap, query)
    if plan == nil then
        return { name = nil }
    end
    local members = party.members(snap)
    local crew    = nil
    if #members > 0 then
        local rested = {}
        for _, member in ipairs(members) do
            table.insert(rested, score.rested(member, ground.mode(member)))
        end
        crew = ground.crew(rested, query)
    end
    local ranked = {}
    for index, entry in ipairs(ground.ranked(snap, query, ground.mode(snap), crew)) do
        if index > 8 then
            break
        end
        local floors = {}
        for _, floor in ipairs(entry.candidate.floors) do
            table.insert(floors, {
                map = floor.map,
                rate = floor.rate // 1,
                crowd = floor.crowd,
                need = floor.need // 1,
                margin = floor.margin,
                drain = floor.drain,
                pain = floor.pain,
                targets = floor.targets
            })
        end
        table.insert(ranked, {
            name = entry.candidate.bundle.name,
            rate = entry.rate // 1,
            score = entry.score // 1,
            floors = floors
        })
    end
    local doses = {}
    for _, kind in ipairs({ "mp", "hp" }) do
        for _, dose in ipairs(recovery.doses(snap, kind)) do
            table.insert(doses, { kind = kind, name = dose.name, amount = dose.amount, count = dose.count })
        end
    end
    local refill               = ground.choose(snap, query, "REFILL")
    local dry                  = ground.choose(snap, query, "DRY")
    local supply               = require("brain.tasks.supply")
    local choice, count, delta = supply.order(snap, query)
    return {
        name = plan.name,
        rate = plan.rate,
        drain = plan.drain,
        wear = plan.wear,
        need = plan.need,
        refill = refill ~= nil and { name = refill.name, rate = refill.rate, drain = refill.drain, need = refill.need } or
            nil,
        dry = dry ~= nil and { name = dry.name, rate = dry.rate, drain = dry.drain, need = dry.need } or nil,
        order = choice ~= nil and { name = choice.name, count = count, delta = delta } or nil,
        least = recovery.least(snap),
        mana = require("brain.policies.value").mana(snap, query),
        money = snap.self.money,
        bag_used = snap.bag.used,
        bag_capacity = snap.bag.capacity,
        doses = doses,
        reserve = recovery.reserve(snap),
        ranked = ranked,
        shunned = danger.shunned(snap, query)
    }
end

function M.survey(snap, query)
    local rested  = score.rested(snap)
    local ability = score.capability(rested, query)
    local members = party.members(snap)
    local crew    = nil
    if #members > 0 then
        local list = {}
        for _, member in ipairs(members) do
            table.insert(list, score.rested(member, ground.mode(member)))
        end
        crew = ground.crew(list, query)
    end
    local rows = {}
    for index, bundle in ipairs(hunting.bundles) do
        local floor = bundle.floors[1]
        local exp, hp, worst, total = 0, 0, 0, 0
        for _, entry in ipairs(floor.mobs) do
            local mob = hunting.mobs[entry.id]
            if mob ~= nil then
                exp = exp + entry.count * (mob.exp or 0)
                hp = hp + entry.count * (mob.hp or 0)
                if mob.attack == "AGGRESSIVE" then
                    worst = math.max(worst, mob.damage or 0)
                    total = total + entry.count * (mob.damage or 0) / math.max(mob.speed or 1000, 1) * ((mob.hp or 0))
                end
            end
        end
        local result = ground.evaluate(rested, query, floor, ability, crew)
        table.insert(rows, {
            index = index,
            name = bundle.name,
            worst = worst,
            load = hp > 0 and total / hp or 0,
            hp = hp,
            efficiency = hp > 0 and exp / hp or 0,
            safe = result.safe,
            rate = result.rate
        })
    end
    local report = { bundles = #rows }
    local function order(field, label)
        table.sort(rows, function(left, right)
            if left[field] ~= right[field] then
                return left[field] < right[field]
            end
            return left.index < right.index
        end)
        local first_unsafe, last_safe, safe_count = nil, nil, 0
        for position, row in ipairs(rows) do
            if row.safe then
                safe_count = safe_count + 1
                last_safe = position
            elseif first_unsafe == nil then
                first_unsafe = position
            end
        end
        local inversions = 0
        for position, row in ipairs(rows) do
            if row.safe and first_unsafe ~= nil and position > first_unsafe then
                inversions = inversions + 1
            end
        end
        report[label] = { first_unsafe = first_unsafe, last_safe = last_safe, safe = safe_count, inversions = inversions }
    end
    order("worst", "by_damage")

    order("load", "by_load")
    local best = nil
    for _, row in ipairs(rows) do
        if row.safe and (best == nil or row.rate > best.rate) then
            best = row
        end
    end
    report.best = best ~= nil and
        { name = best.name, rate = best.rate // 1, efficiency = best.efficiency, worst = best.worst } or nil
    table.sort(rows, function(left, right)
        return left.efficiency > right.efficiency
    end)
    local must, position_of_best, seen_safe = 0, nil, 0
    for _, row in ipairs(rows) do
        if row.safe then
            seen_safe = seen_safe + 1
            if best ~= nil and row.name == best.name then
                position_of_best = seen_safe
            end
        end
    end
    report.best_in_efficiency_order = position_of_best
    report.top_rate_share = {}
    local kept = {}
    for _, row in ipairs(rows) do
        if row.safe then
            table.insert(kept, row)
        end
    end
    for _, limit in ipairs({ 5, 10, 20, 40 }) do
        local top = 0
        for position = 1, math.min(limit, #kept) do
            top = math.max(top, kept[position].rate)
        end
        table.insert(report.top_rate_share,
            { limit = limit, share = best ~= nil and best.rate > 0 and top / best.rate or 0 })
    end
    return report
end

function M.mob(id)
    return hunting.mobs[id]
end

return M
