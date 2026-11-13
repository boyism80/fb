local world  = require("sim.world.suite")
local roster = require("sim.world.roster")
local json   = require("sim.world.json")

local M = {}

M.COSTS = "logs/cluster/lane-costs.json"

local RECENT = 240
local SAMPLE = 50
local STALL  = 30000
local BOUNCE = 1000
local STRIDE = 1000

local function overlay(base, changes)
    return setmetatable(changes, { __index = base })
end

local function folder(path)
    if package.config:sub(1, 1) == "\\" then
        os.execute(string.format('mkdir "%s" 2>nul', (path:gsub("/", "\\"))))
    else
        os.execute(string.format('mkdir -p "%s"', path))
    end
end

local function staged(scene, name)
    local line  = nil
    local count = 0
    for text in io.lines(scene.file) do
        count = count + 1
        if count == scene.row or scene.row == nil or scene.row == 0 then
            line = text
        end
    end
    local record = assert(load(assert(line, "no row " .. tostring(scene.row) .. " in " .. scene.file)))()
    local snap   = record.snap or record
    local stood  = { { class = snap.self.class, world = snap.world } }
    for _, member in ipairs(snap.party or {}) do
        table.insert(stood, { class = member.self.class, world = member.world })
    end
    local spots = {}
    for _, team in ipairs(roster.team.teams) do
        for index, want in ipairs(team.name == name and team.members or {}) do
            for _, entry in ipairs(stood) do
                if spots[index] == nil and entry.taken == nil and entry.class == want.class and entry.world.x ~= nil then
                    spots[index] = { map = entry.world.map, x = entry.world.x, y = entry.world.y }
                    entry.taken  = true
                end
            end
        end
    end
    local mobs = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil then
            table.insert(mobs, { map = snap.world.map, id = mob.id, x = mob.x, y = mob.y })
        end
    end
    return spots, mobs
end

local function planned(home)
    local asked = (json.read(home .. "/params.json") or {}).sim
    assert(type(asked) == "table", "sim lanes need a sim table in " .. home .. "/params.json")
    local plan = {}
    for key, value in pairs(asked) do
        plan[key] = value
    end
    if plan.teams == nil then
        plan.teams = {}
        for _, team in ipairs(roster.team.teams) do
            table.insert(plan.teams, team.name)
        end
    end
    if plan.trace == nil then
        plan.trace = plan.profile ~= true
    end
    if plan.scene ~= nil then
        plan.start, plan.placed = staged(plan.scene, plan.teams[1])
    end
    plan.home     = home
    plan.seeds    = plan.seeds or { 0, 1, 2 }
    plan.duration = math.floor((plan.minutes or 0) * 60000)
    plan.guard    = plan.guard ~= false
    plan.recent   = plan.recent or RECENT
    plan.stall    = plan.stall or STALL
    plan.bounce   = plan.bounce or BOUNCE
    plan.queue    = plan.fixed ~= true and home .. "/queue" or nil
    plan.alarm    = plan.guard and home .. "/alarm.jsonl" or nil
    plan.sample   = plan.corpus and (plan.sample or SAMPLE) or nil
    plan.corpus   = plan.corpus and home .. "/corpus" or nil
    plan.stop     = plan.halt and home .. "/stop" or nil
    plan.output   = home .. "/episodes"
    plan.costs    = json.read(M.COSTS) or {}
    for _, path in ipairs({ plan.queue, plan.corpus, plan.output }) do
        folder(path)
    end
    return plan
end

local function jobs(plan)
    local found = {}
    local index = 0
    local span  = "|" .. tostring(plan.duration)
    for _, seed in ipairs(plan.seeds) do
        for _, name in ipairs(plan.teams) do
            index = index + 1
            table.insert(found, { index = index, seed = seed, team = name, key = "team|" .. name .. span })
        end
        if plan.solo then
            for profile = 1, roster.solo_count do
                index = index + 1
                table.insert(found, { index = index, seed = seed, profile = profile, key = "solo|" .. profile .. span })
            end
        end
    end
    return found
end

local function ordered(plan, list)
    local costs = plan.costs
    local known = {}
    for _, cost in pairs(costs) do
        table.insert(known, cost)
    end
    table.sort(known)
    local usual = known[(#known + 1) // 2] or 0
    table.sort(list, function(left, right)
        local a, b = costs[left.key] or usual, costs[right.key] or usual
        if a ~= b then
            return a > b
        end
        return left.index < right.index
    end)
    return list
end

local function claim(plan, lane, job)
    if plan.queue == nil then
        return (job.index - 1) % plan.lanes == lane - 1
    end
    local mine = string.format("%s/%d.lane%d", plan.queue, job.index, lane)
    local file = io.open(mine, "w")
    if file == nil then
        return false
    end
    file:close()
    local taken = os.rename(mine, string.format("%s/%d", plan.queue, job.index))
    if not taken then
        os.remove(mine)
    end
    return taken == true
end

local function stopped(plan)
    local file = plan.stop ~= nil and io.open(plan.stop, "rb") or nil
    if file ~= nil then
        file:close()
    end
    return file ~= nil
end

local function perform(plan, job, progress, report)
    local bots     = {}
    local options  = {
        seed_base = job.seed,
        tag = job.index,
        exp_multiplier = plan.exp,
        guard = plan.guard,
        halt = plan.halt,
        stop = plan.stop,
        stall = plan.stall,
        bounce = plan.bounce,
        audit = plan.audit,
        alarm = plan.alarm,
        recent = plan.recent,
        sample = plan.sample,
        corpus = plan.corpus,
        output = plan.output,
        arrange = plan.start ~= nil and function()
            local spots = {}
            for index, spot in ipairs(plan.start) do
                spots[index] = { map = spot.map, x = spot.x, y = spot.y }
            end
            return spots
        end or nil,
        placed = plan.placed,
        record = function(bot)
            table.insert(bots, bot)
        end,
        raised = function(rule, bot)
            progress.alarms = progress.alarms + 1
            progress.alarm  = rule .. " " .. bot
            report()
        end,
    }
    options.passed = function(clock)
        progress.clock = clock
        progress.span  = options.duration
        report()
    end
    if job.team ~= nil then
        options.only     = { [job.team] = true }
        options.duration = plan.duration > 0 and plan.duration or roster.team.duration
        world.teams(overlay(roster.team, options))
    else
        options.only     = { [job.profile] = true }
        options.acts     = plan.trace
        options.duration = plan.duration > 0 and plan.duration or roster.solo.duration
        world.run(overlay(roster.solo, options))
    end
    return bots
end

local function timed(stepped)
    local decide = require("brain.decide")
    local choose = decide.choose
    local cost   = { count = 0, spent = 0, steps = 0 }
    local inside = false
    if stepped then
        debug.sethook(function()
            if inside then
                cost.steps = cost.steps + 1
            end
        end, "", STRIDE)
    end
    decide.choose = function(...)
        local began           = os.clock()
        inside                = true
        local decision, later = choose(...)
        inside                = false
        cost.spent            = cost.spent + os.clock() - began
        cost.count            = cost.count + 1
        return decision, later
    end
    return cost
end

function M.run(lane, ctx)
    local home                         = assert(os.getenv("FB_RUN_DIR"), "sim lanes need FB_RUN_DIR")
    local plan                         = planned(home)
    require("brain.data.combat").trace = plan.trace == true
    local prof                         = plan.profile == true and require("sim.world.prof") or nil
    if prof ~= nil then
        prof.install()
    end
    local cost     = timed(plan.steps == true)
    local tap      = os.getenv("LANE_TAP")
    local close    = tap ~= nil and dofile(tap) or nil
    local finished = {}
    local list     = ordered(plan, jobs(plan))
    local progress = { jobs = #list, done = 0, alarms = 0 }
    local function report()
        ctx:progress(json.encode(progress))
    end
    for _, job in ipairs(list) do
        if stopped(plan) == false and claim(plan, lane, job) then
            progress.job  = job.team or "solo " .. job.profile
            progress.seed = job.seed
            report()
            local started = os.time()
            local bots    = perform(plan, job, progress, report)
            table.insert(finished, {
                index = job.index,
                key = job.key,
                team = job.team,
                profile = job.profile,
                seed = job.seed,
                seconds = os.time() - started,
                bots = bots,
            })
            progress.done  = progress.done + 1
            progress.job   = nil
            progress.seed  = nil
            progress.clock = nil
            progress.span  = nil
            report()
        end
    end
    if close ~= nil then
        close()
    end
    json.write(string.format("%s/lane-%02d.json", home, lane), {
        lane = lane,
        jobs = finished,
        decisions = cost.count,
        decide = cost.spent,
        steps = cost.steps,
        profile = prof ~= nil and prof.report() or nil,
    })
    return true
end

return M
