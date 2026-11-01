local alert   = require("brain.policies.alert")
local failure = require("brain.policies.failure")
local states  = require("brain.data.states")

local M = {}

local function module(name)
    return require("brain.tasks." .. string.lower(name))
end

function M.first(brain, bot, current)
    local seen = brain:observe(bot)
    for _, state in ipairs(states) do
        for _, name in ipairs(state.tasks) do
            if name == current then
                return nil
            end

            local task = module(name)
            if task.wanted(brain, bot, seen) and failure.allowed(brain, bot, name) then
                return {
                    state  = state.name,
                    task   = name,
                    step   = task.step ~= nil and task.step(bot) or 0,
                    reason = name,
                }
            end
        end
    end
    return nil
end

function M.decide(brain, bot)
    alert.watch(brain, bot)
    return M.first(brain, bot, nil) or { state = "PREPARE", reason = "idle" }
end

return M
