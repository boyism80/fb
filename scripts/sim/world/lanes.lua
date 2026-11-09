local world  = require("sim.world.suite")
local roster = require("sim.world.roster")

local M = {}

local function overlay(base, changes)
    return setmetatable(changes, { __index = base })
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
    local costs = plan.costs or {}
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

local function done(plan, job, seconds)
    if plan.queue == nil then
        return
    end
    local file = io.open(string.format("%s/%d.done", plan.queue, job.index), "w")
    if file ~= nil then
        file:write(job.key, "\t", tostring(seconds), "\n")
        file:close()
    end
end

local function perform(plan, job)
    local options = {
        seed_base = job.seed,
        tag = job.index,
        exp_multiplier = plan.exp,
        guard = plan.guard,
        halt = plan.halt,
        stall = plan.stall,
        bounce = plan.bounce,
        audit = plan.audit,
        alarm = plan.alarm,
    }
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
end

function M.run(lane)
    local plan                         = require("sim.world.lane_plan")
    require("brain.data.combat").trace = plan.trace == true
    local prof                         = plan.profile == true and require("sim.world.prof") or nil
    if prof ~= nil then
        prof.install()
    end
    for _, job in ipairs(ordered(plan, jobs(plan))) do
        if claim(plan, lane, job) then
            local started = os.time()
            perform(plan, job)
            done(plan, job, os.time() - started)
        end
    end
    if prof ~= nil then
        prof.report()
    end
    return true
end

return M
