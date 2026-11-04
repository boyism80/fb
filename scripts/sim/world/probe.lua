local ground   = require("brain.policies.ground")
local escort   = require("brain.policies.escort")
local party    = require("brain.policies.party")
local score    = require("brain.policies.score")
local survival = require("brain.policies.survival")
local danger   = require("brain.policies.danger")
local hunting  = require("brain.data.hunting")
local facts    = require("sim.facts")
local driver   = require("sim.world.driver")

local M = {}

local function floor(map)
    for _, bundle in ipairs(hunting.bundles) do
        for _, entry in ipairs(bundle.floors) do
            if entry.map == map then
                return entry
            end
        end
    end
    return nil
end

local function knows(snap, names)
    local known = {}
    for _, spell in ipairs(snap.spells or {}) do
        known[spell.name] = true
    end
    for _, name in ipairs(names or {}) do
        if known[name] == nil then
            return false
        end
    end
    return true
end

local function find(records, want, taken)
    for index, record in ipairs(records) do
        local snap = record.snap
        if taken[index] == nil and snap.self.class == want.class and snap.self.level >= want.low and
            snap.self.level <= want.high and knows(snap, want.spells) then
            taken[index] = true
            return snap
        end
    end
    return nil
end

local function sample(records, class, low, high)
    local best = nil
    local mid  = (low + high) / 2
    for _, record in ipairs(records) do
        local snap = record.snap
        if snap.self.class == class and snap.self.level >= low and snap.self.level <= high and
            (best == nil or math.abs(snap.self.level - mid) < math.abs(best.self.level - mid)) then
            best = snap
        end
    end
    return best
end

local function describe(plan)
    if plan == nil then
        return "none"
    end
    return string.format("%s floors=%d rate=%.0f margin=%.2f need=%.0f drain=%.1f", plan.name, #plan.floors, plan.rate,
        plan.margin or 0, plan.need or 0, plan.drain or 0)
end

function M.survey(options)
    local data = require(options.data)
    local mind = brain.new()
    for _, class in ipairs({ "WARRIOR", "ROGUE", "MAGE", "POET" }) do
        for low = 1, 91, 10 do
            local high = low + 9
            local found = sample(data.records, class, low, high)
            if found ~= nil then
                local snap  = driver.prepare(found, {})
                local bound = facts.bind(mind, snap)
                local solo  = ground.choose(snap, bound)
                local ranks = {}
                for index, entry in ipairs(ground.ranked(snap, bound)) do
                    if index <= 3 then
                        table.insert(ranks, string.format("%s=%.0f", entry.candidate.bundle.name, entry.rate))
                    end
                end
                local mate = sample(data.records, class == "POET" and "WARRIOR" or "POET", low, high)
                local duo  = "none"
                if mate ~= nil then
                    local other         = driver.prepare(mate, {})
                    snap.self.grouping  = true
                    other.self.grouping = true
                    local plan, front   = ground.party({ snap, other }, facts.bind(mind, snap))
                    duo                 = describe(plan) ..
                        " front=" .. tostring(front) .. " mate=lv" .. other.self.level
                end
                log("debug",
                    string.format("[sim:survey] %s lv%d hp=%d mp=%d solo=%s | top=%s | duo=%s", class, snap.self.level,
                        snap.self.max_hp, snap.self.max_mp or 0, describe(solo), table.concat(ranks, ","), duo))
            end
        end
    end
    local shown = 0
    for _, record in ipairs(data.records) do
        local snap = record.snap
        if snap.self.class == "WARRIOR" and snap.self.level >= 63 and snap.self.level <= 79 and shown < 3 then
            local known = false
            local fewer = {}
            for _, spell in ipairs(snap.spells or {}) do
                known = known or spell.name == "건곤대나이"
                if spell.name ~= "사중공격" and spell.name ~= "오중공격" then
                    table.insert(fewer, spell)
                end
            end
            if known then
                shown = shown + 1
                local clone = {}; for k, v in pairs(snap) do clone[k] = v end; clone.spells = fewer; local prepared =
                    driver.prepare(clone, {})
                prepared.spells = fewer
                local bound     = facts.bind(mind, prepared)
                local taught    = { table.unpack(fewer) }
                table.insert(taught, { name = "사중공격", slot = 0 })
                local after  = setmetatable({ spells = taught, hypothesis = "사중공격", actual = prepared },
                    { __index = prepared })
                local plain  = score.rested(prepared)
                local learnt = score.rested(after)
                local source = floor(957)
                local r0     = ground.evaluate(plain, bound, source, score.capability(plain, bound), nil)
                local r1     = ground.evaluate(learnt, bound, source, score.capability(learnt, bound), nil)
                log("debug",
                    string.format("[sim:survey:learn] lv%d rate %.0f -> %.0f safe %s -> %s margin %.2f -> %.2f",
                        prepared.self.level, r0.rate, r1.rate, tostring(r0.safe), tostring(r1.safe), r0.margin, r1
                        .margin))
                local combat = require("brain.policies.combat")
                for _, entry in ipairs(source.mobs) do
                    local mob = hunting.mobs[entry.id]
                    local o0  = combat.outcome(plain, bound, mob)
                    local o1  = combat.outcome(learnt, bound, mob)
                    log("debug",
                        string.format(
                            "[sim:survey:learn]   %s hp=%d time %.0f -> %.0f taken %.0f -> %.0f spent %.0f -> %.0f", mob
                            .name, mob.hp, o0.time, o1.time, o0.taken, o1.taken, o0.spent or 0, o1.spent or 0))
                end
            end
        end
    end
    return true
end

local function bundle_named(name)
    for _, bundle in ipairs(hunting.bundles) do
        if bundle.name == name then
            return bundle
        end
    end
    return nil
end

local function learned(snap, name)
    local clone = {}
    for key, entry in pairs(snap) do
        clone[key] = entry
    end
    clone.spells = { table.unpack(snap.spells or {}) }
    local known = false
    for _, spell in ipairs(clone.spells) do
        known = known or spell.name == name
    end
    if known == false then
        table.insert(clone.spells, { name = name, slot = 0 })
    end
    return clone
end

local function inspect_floor(tag, mind, lead, others, bundle)
    local bound   = facts.bind(mind, lead)
    local rested  = score.rested(lead, "REFILL")
    local ability = score.capability(rested, bound)
    local crew    = nil
    if #others > 0 then
        local list = {}
        for _, other in ipairs(others) do
            table.insert(list, score.rested(other, "REFILL"))
        end
        crew = ground.crew(list, bound)
    end
    local floor  = bundle.floors[1]
    local result = ground.evaluate(rested, bound, floor, ability, crew)
    log("debug",
        string.format(
            "[sim:gate] %s %s lv%d hp=%d mp=%d def=%d crewheal=%.0f %s safe=%s rate=%.0f margin=%.2f crowd=%.2f need=%.0f drain=%.1f",
            tag, lead.self.class,
            lead.self.level, rested.self.max_hp, rested.self.max_mp or 0, rested.self.phydef or 0,
            crew ~= nil and crew.heal or 0, bundle.name, tostring(result.safe), result.rate,
            result.margin == math.huge and -1 or result.margin, result.crowd or 0, result.need, result.drain))
    for _, entry in ipairs(floor.mobs) do
        local mob = hunting.mobs[entry.id]
        if mob ~= nil then
            local context = escort.context(rested, bound, mob, crew, floor.density, ability.refill.hp)
            context.crowd = { extra = result.crowd or 0, pressure = 0 }
            local fought  = survival.fight(rested, bound, mob, context)
            log("debug",
                string.format(
                    "[sim:gate]    %s x%d %s dmg=%d hp=%d time=%.0f taken=%.0f healed=%.0f ending=%.0f burst=%.0f deadly=%s support=%.0f share=%.2f",
                    mob.name, entry.count, mob.attack,
                    mob.damage or 0, mob.hp or 0, fought.time, fought.taken, fought.healed, fought.ending,
                    ground.burst(rested, mob, { extra = result.crowd or 0 }),
                    tostring(danger.deadly(rested, bound, mob, false)), context.support or 0, context.share or 0))
        end
    end
end

function M.gates(options)
    local data = require(options.data)
    local mind = brain.new()
    for _, set in ipairs(options.sets) do
        local attacker         = driver.prepare(sample(data.records, set.class, set.low, set.high), {})
        local poet             = driver.prepare(learned(sample(data.records, "POET", set.low, set.high), "생명의기원"), {})
        local plain            = driver.prepare(sample(data.records, "POET", set.low, set.high), {})
        attacker.self.grouping = true
        poet.self.grouping     = true
        for _, name in ipairs(set.bundles) do
            local bundle = bundle_named(name)
            inspect_floor("생명없음", mind, attacker, { plain }, bundle)
            inspect_floor("생명있음", mind, attacker, { poet }, bundle)
            inspect_floor("도사", mind, poet, { attacker }, bundle)
        end
        local bound = facts.bind(mind, attacker)
        local plan  = ground.party({ attacker, poet }, bound)
        log("debug",
            string.format("[sim:gate] 선택 %s lv%d + 도사 lv%d(생명) -> %s rate=%.0f", set.class, attacker.self.level,
                poet.self.level, plan ~= nil and plan.name or "none", plan ~= nil and plan.rate or 0))
        local known = bound.reach({}, {}) or {}
        for _, name in ipairs(set.bundles) do
            log("debug",
                string.format("[sim:gate] 접근 %s reach=%s", name,
                    tostring(known[bundle_named(name).floors[1].map] == true)))
        end
    end
    return true
end

function M.run(options)
    local data  = require(options.data)
    local mind  = brain.new()
    local taken = {}
    for _, team in ipairs(options.teams) do
        local snaps = {}
        for _, want in ipairs(team.members) do
            local snap = driver.prepare(find(data.records, want, taken), {})
            snap.self.grouping = true
            table.insert(snaps, snap)
        end
        if team.bundle ~= nil then
            local members = {}
            for _, member in ipairs(snaps) do
                table.insert(members, member)
            end
            local held = ground.hold(members, facts.bind(mind, snaps[1]), team.bundle)
            log("debug", string.format("[sim:probe] %s hold %s -> %s", team.name, team.bundle,
                held ~= nil and (#held.floors .. " floors rate " .. string.format("%.1f", held.rate)) or "nil"))
            for index, member in ipairs(members) do
                local bound   = facts.bind(mind, member)
                local rested  = score.rested(member, ground.mode(member))
                local ability = score.capability(rested, bound)
                local others  = {}
                for other, entry in ipairs(members) do
                    if other ~= index then
                        table.insert(others, score.rested(entry, ground.mode(entry)))
                    end
                end
                local crew = ground.crew(others, bound)
                for _, map in ipairs(team.maps) do
                    local source = floor(map)
                    local result = ground.evaluate(rested, bound, source, ability, crew)
                    log("debug", string.format("[sim:probe]   lead=%s map=%d safe=%s rate=%.1f margin=%.2f shunned=%s",
                        member.self.class, map, tostring(result.safe), result.rate, result.margin,
                        table.concat(danger.shunned(member, bound), ",")))
                end
            end
        end
        for index, snap in ipairs(snaps) do
            local bound  = facts.bind(mind, snap)
            local others = {}
            for other, member in ipairs(snaps) do
                if other ~= index then
                    table.insert(others, escort.member(party.profile(member, facts.bind(mind, member))))
                end
            end
            local crew    = #others > 0 and ground.crew(others, bound) or nil
            local rested  = score.rested(snap, ground.mode(snap))
            local ability = score.capability(rested, bound)
            log("debug", string.format("[sim:probe] %s %s lv%d hp=%d mp=%d heal_others=%.1f mp_rate=%.1f crew_heal=%.1f",
                team.name, snap.self.class, snap.self.level, rested.self.max_hp, rested.self.max_mp or 0,
                ability.hps.others, ability.refill.mp, crew ~= nil and crew.heal or 0))
            for _, map in ipairs(team.maps) do
                local source = floor(map)
                if source ~= nil then
                    local result = ground.evaluate(rested, bound, source, ability, crew)
                    log("debug", string.format(
                        "[sim:probe]   map=%d safe=%s rate=%.1f kills=%.4f deaths=%.6f margin=%.2f wear=%.1f crowd=%.2f need=%.0f",
                        map, tostring(result.safe), result.rate, result.kills, result.deaths, result.margin, result.wear,
                        result.crowd or 0, result.need))
                    for _, entry in ipairs(source.mobs) do
                        local mob = hunting.mobs[entry.id]
                        if mob ~= nil then
                            local context = escort.context(rested, bound, mob, crew, source.density, ability.refill.hp)
                            local fought  = survival.fight(rested, bound, mob, context)
                            log("debug", string.format(
                                "[sim:probe]     %s x%d %s time=%.0f taken=%.0f healed=%.0f ending=%.0f p_die=%.4f aid=%.3f speed=%.6f deadly=%s burst=%.0f",
                                mob.name, entry.count, mob.attack, fought.time, fought.taken, fought.healed,
                                fought.ending, fought.p_die,
                                context.aid or 0, context.speed or 0, tostring(danger.deadly(rested, bound, mob, false)),
                                ground.burst(rested, mob, { extra = result.crowd or 0 })))
                        end
                    end
                end
            end
        end
    end
    return true
end

return M
