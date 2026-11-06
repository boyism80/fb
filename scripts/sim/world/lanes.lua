local world  = require("sim.world.suite")
local roster = require("sim.world.roster")

local M = {}

local function overlay(base, changes)
    return setmetatable(changes, { __index = base })
end

function M.run(lane)
    local plan                         = require("sim.world.lane_plan")
    require("brain.data.combat").trace = plan.trace == true
    local prof                         = plan.profile == true and require("sim.world.prof") or nil
    if prof ~= nil then
        prof.install()
    end
    local index = 0
    for _, seed in ipairs(plan.seeds) do
        for _, name in ipairs(plan.teams) do
            index = index + 1
            if (index - 1) % plan.lanes == lane - 1 then
                world.teams(overlay(roster.team, {
                    seed_base = seed,
                    only = { [name] = true },
                    tag = index,
                    exp_multiplier = plan.exp,
                    duration = plan.duration > 0 and plan.duration or roster.team.duration,
                }))
            end
        end
        if plan.solo then
            for profile = 1, roster.solo_count do
                index = index + 1
                if (index - 1) % plan.lanes == lane - 1 then
                    world.run(overlay(roster.solo, {
                        seed_base = seed,
                        only = { [profile] = true },
                        tag = index,
                        acts = plan.trace,
                        exp_multiplier = plan.exp,
                        duration = plan.duration > 0 and plan.duration or roster.solo.duration,
                    }))
                end
            end
        end
    end
    if prof ~= nil then
        prof.report()
    end
    return true
end

return M
