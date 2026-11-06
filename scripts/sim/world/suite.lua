local driver = require("sim.world.driver")

local M = {}

local function pick(records, options)
    if options.start then
        local chosen = {}
        for _, record in ipairs(records) do
            if options.classes[record.snap.self.class] then
                table.insert(chosen, record.snap)
            end
        end
        return chosen
    end
    local chosen = {}
    local counts = {}
    for _, record in ipairs(records) do
        local snap = record.snap
        if options.classes[snap.self.class] then
            local key = snap.self.class .. "|" .. ((snap.self.level - 1) // options.band)
            counts[key] = counts[key] or 0
            if counts[key] < options.per then
                counts[key] = counts[key] + 1
                table.insert(chosen, snap)
            end
        end
    end
    table.sort(chosen, function(left, right)
        if left.self.class ~= right.self.class then
            return left.self.class < right.self.class
        end
        return left.self.level < right.self.level
    end)
    return chosen
end

local function open(directory, tag)
    local path = string.format("%s/world_%d%s.jsonl", directory, os.time(), tag ~= nil and ("_" .. tag) or "")
    local file = io.open(path, "wb")
    if file == nil then
        os.execute(string.format('mkdir "%s"', directory:gsub("/", "\\")))
        file = assert(io.open(path, "wb"))
    end
    return file, path
end

function M.run(options)
    local data         = require(options.data)
    local mind         = brain.new()
    local output, path = open(options.output, options.tag)
    local profiles     = pick(data.records, options)
    if options.only ~= nil then
        local kept = {}
        for index, profile in ipairs(profiles) do
            if options.only[index] then
                table.insert(kept, profile)
            end
        end
        profiles = kept
    end
    if options.strip ~= nil then
        for index, profile in ipairs(profiles) do
            local gear = {}
            for _, slot in ipairs(profile.gear or {}) do
                table.insert(gear, options.strip[slot.type] and { type = slot.type, names = {} } or slot)
            end
            local stripped = {}
            for key, value in pairs(profile) do
                stripped[key] = value
            end
            stripped.gear   = gear
            profiles[index] = stripped
        end
    end
    local finished = 0
    for index, profile in ipairs(profiles) do
        local settings   = setmetatable({ seed = index + (options.seed_base or 0) * 100, output = output },
            { __index = options })
        local ok, result = pcall(driver.run, mind, profile, settings)
        if ok == false then
            log("warn", string.format("[sim:world] %s lv%d failed: %s", profile.self.class, profile.self.level,
                tostring(result)))
        elseif result.skipped ~= nil then
            log("debug", string.format("[sim:world] %s lv%d skipped %s map=%s", profile.self.class, profile.self.level,
                result.skipped, tostring(result.map)))
        else
            finished = finished + 1
            local tasks = {}
            for name, count in pairs(result.tasks) do
                table.insert(tasks, string.format("%s=%d", name, count))
            end
            table.sort(tasks)
            local idle = {}
            for name, time in pairs(result.idle) do
                table.insert(idle, string.format("%s=%.0fs", name, time / 1000))
            end
            table.sort(idle)
            local missing = {}
            for name, count in pairs(result.unsupported) do
                table.insert(missing, string.format("%s=%d", name, count))
            end
            table.sort(missing)
            log("debug", string.format(
                "[sim:world] %s lv%d ground=%s map=%d time=%.0fs died=%s left=%s kills=%d exp=%d swings=%d hits=%d taken=%d decisions=%d tasks=%s idle=%s unsupported=%s",
                result.class, result.level, tostring(result.ground), result.map, result.time / 1000,
                tostring(result.died), tostring(result.departed), result.kills, result.exp, result.swings, result.hits,
                result.taken, result.decisions, table.concat(tasks, ","), table.concat(idle, ","),
                table.concat(missing, ",")))
        end
        output:flush()
    end
    output:close()
    log("debug", string.format("[sim:world] profiles=%d finished=%d output=%s", #profiles, finished, path))
    return finished > 0
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

local function lacks(snap, names)
    for _, name in ipairs(names or {}) do
        for _, spell in ipairs(snap.spells or {}) do
            if spell.name == name then
                return false
            end
        end
    end
    return true
end

local function force(mind, snap)
    for _, slot in ipairs(snap.gear or {}) do
        local model = slot.type == "WEAPON" and slot.names[1] ~= nil and mind:item(slot.names[1]) or nil
        if model ~= nil and model.small ~= nil then
            return model.small.max or 0
        end
    end
    return 0
end

local CLASSES = { ["전사"] = "WARRIOR", ["도적"] = "ROGUE", ["주술사"] = "MAGE", ["도사"] = "POET" }

local START = { map = 22, x = 16, y = 103 }

local function dressed(records, name, money, mind)
    local spec = nil
    for _, entry in ipairs(require("integration.scenarios.brain.state.profiles")) do
        if entry.name == name then
            spec = entry
        end
    end
    local base = nil
    local class = CLASSES[spec.class]
    for _, record in ipairs(records) do
        if record.snap.self.class == class and (base == nil or
                math.abs(record.snap.self.level - spec.level) < math.abs(base.self.level - spec.level)) then
            base = record.snap
        end
    end
    local me = {}
    for key, entry in pairs(base.self) do
        me[key] = entry
    end
    me.level, me.hp, me.max_hp, me.mp, me.max_mp = spec.level, spec.hp, spec.hp, spec.mp, spec.mp
    me.str, me.int, me.dex, me.money = spec.str, spec.int, spec.dex, money
    me.exp = 0
    local spells = {}
    for slot, spell in ipairs(spec.spells) do
        spells[slot] = { slot = slot, name = spell }
    end
    local gear = {}
    for _, slot in ipairs(base.gear) do
        local worn = spec.gear[slot.type] or nil
        table.insert(gear, { type = slot.type, names = worn ~= nil and { worn } or {} })
    end
    me.hit, me.dam, me.phydef = 0, 0, 0
    for _, slot in ipairs(gear) do
        local model = slot.names[1] ~= nil and mind:item(slot.names[1]) or nil
        if model ~= nil then
            me.phydef = me.phydef - (model.defense or 0)
            me.hit    = me.hit + (model.hit or 0)
            me.dam    = me.dam + (model.bonus or 0)
            me.max_hp = me.max_hp + (model.hp or 0)
        end
    end
    local snap = {}
    for key, entry in pairs(base) do
        snap[key] = entry
    end
    snap.self, snap.spells, snap.gear = me, spells, gear
    snap.bag = { items = {}, used = 0, capacity = base.bag.capacity }
    snap.buffs = {}
    snap.world = { map = START.map, x = START.x, y = START.y }
    return snap
end

local function find(records, want, taken, mind)
    local best = nil
    for index, record in ipairs(records) do
        local snap = record.snap
        if taken[index] == nil and snap.self.class == want.class and snap.self.level >= want.low and
            snap.self.level <= want.high and knows(snap, want.spells) and lacks(snap, want.without) then
            if want.armed ~= true then
                taken[index] = true
                return snap
            elseif best == nil or force(mind, snap) > force(mind, records[best].snap) then
                best = index
            end
        end
    end
    if best ~= nil then
        taken[best] = true
        return records[best].snap
    end
    return nil
end

local function summary(result)
    local parts = {}
    for _, group in ipairs({ { "tasks", result.tasks }, { "casts", result.casts }, { "used", result.used },
        { "idle", result.idle } }) do
        local entries = {}
        for name, value in pairs(group[2] or {}) do
            table.insert(entries, string.format("%s=%s", name, type(value) == "number" and
                (group[1] == "idle" and string.format("%.0fs", value / 1000) or tostring(value)) or tostring(value)))
        end
        table.sort(entries)
        table.insert(parts, group[1] .. "=" .. table.concat(entries, ","))
    end
    return string.format("%s lv%d died=%s grouped=%s kills=%d exp=%d dealt=%d taken=%d %s", result.class,
        result.level, tostring(result.died), tostring(result.grouped), result.kills, result.exp, result.dealt or 0,
        result.taken, table.concat(parts, " "))
end

function M.teams(options)
    local data         = require(options.data)
    local mind         = brain.new()
    local output, path = open(options.output, options.tag)
    local taken        = {}
    local passed       = 0
    for index, team in ipairs(options.teams) do
        local profiles = {}
        local missing  = nil
        for _, want in ipairs(team.members) do
            local snap = want.profile ~= nil and dressed(data.records, want.profile, want.money or 100000, mind) or
                find(data.records, want, taken, mind)
            if snap == nil then
                missing = want.class
                break
            end
            if want.drop ~= nil then
                local kept = {}
                for _, learned in ipairs(snap.spells or {}) do
                    local dropped = false
                    for _, name in ipairs(want.drop) do
                        dropped = dropped or name == learned.name
                    end
                    if dropped == false then
                        table.insert(kept, learned)
                    end
                end
                local copied = {}
                for key, entry in pairs(snap) do
                    copied[key] = entry
                end
                copied.spells = kept
                snap = copied
            end
            table.insert(profiles, { profile = snap, overrides = { grouping = true } })
        end
        if options.only ~= nil and options.only[team.name] == nil then
            goto continue
        end
        if missing ~= nil then
            log("warn", string.format("[sim:team] %s no profile for %s", team.name, missing))
        else
            local settings   = setmetatable(
                { seed = index + (options.seed_base or 0) * 100, output = output, drops = team.drops },
                { __index = options })
            local ok, result = pcall(driver.team, mind, profiles, settings, team.ground)
            if ok == false then
                log("warn", string.format("[sim:team] %s failed: %s", team.name, tostring(result)))
            elseif result.skipped ~= nil then
                log("warn", string.format("[sim:team] %s skipped %s", team.name, result.skipped))
            else
                passed = passed + 1
                for _, entry in ipairs(result) do
                    log("debug",
                        string.format("[sim:team] %s seed=%d %s", team.name, options.seed_base or 0, summary(entry)))
                end
            end
        end
        output:flush()
        ::continue::
    end
    output:close()
    log("debug", string.format("[sim:team] teams=%d finished=%d output=%s", #options.teams, passed, path))
    return passed > 0
end

return M
