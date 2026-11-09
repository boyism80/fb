local failure = require("brain.policies.failure")
local states  = require("brain.data.states")

local M = {}

local function module(name)
    return require("brain.tasks." .. string.lower(name))
end

local function state_of(name)
    for _, state in ipairs(states) do
        for _, task in ipairs(state.tasks) do
            if task == name then
                return state.name
            end
        end
    end
    return "PREPARE"
end

local function postpone(snap, remaining)
    if remaining ~= nil then
        snap.revisit = math.min(snap.revisit or remaining, remaining)
    end
end

local function action_node(name, run)
    return { kind = "act", name = name, run = run }
end

M.node = action_node

local function leaf(name, profitable)
    return action_node(name, function(snap, facts)
        local task = module(name)
        if task.wanted(snap, facts) == false then
            if task.revisit ~= nil then
                postpone(snap, task.revisit(snap, facts))
            end
            return nil
        end
        local remaining = failure.remaining(snap, name)
        if remaining ~= 0 then
            postpone(snap, remaining)
            return nil
        end
        if profitable and task.value ~= nil and task.value(snap, facts) < 0 then
            return nil
        end
        return { name = name, task = task, state = state_of(name) }
    end)
end

function M.select(children, name)
    return { kind = "select", children = children, name = name }
end

function M.when(test, child, name)
    local info  = debug.getinfo(test, "S")
    local label = name or (info.short_src:match("([%w_]+)%.lua$") .. ":" .. info.linedefined)
    return { kind = "when", test = test, child = child, name = label }
end

local function runnable(child)
    if child.run ~= nil then
        return child
    end
    return {
        kind = "act",
        name = child.name,
        inner = child,
        run = function(snap, facts)
            local found, path = run_tree(child, snap, facts)
            if found ~= nil then
                found.via = table.concat(path, ">")
            end
            return found
        end,
    }
end

function M.pick(children, measure, name)
    local wrapped = {}
    for index, child in ipairs(children) do
        wrapped[index] = runnable(child)
    end
    return { kind = "pick", children = wrapped, measure = measure, name = name }
end

function M.postpone(snap, remaining)
    postpone(snap, remaining)
end

function M.compared(snap, facts, found)
    local worth = found.worth
    if worth == nil then
        worth = found.task.value(snap, facts)
    end
    return worth >= 0 and worth or nil
end

function M.act(name, propose, node, worth)
    return action_node(node or name, function(snap, facts)
        local remaining = failure.remaining(snap, name)
        if remaining ~= 0 then
            postpone(snap, remaining)
            return nil
        end
        local action = propose(snap, facts)
        if action == nil then
            return nil
        end
        return {
            name = name,
            task = module(name),
            state = state_of(name),
            action = action,
            worth = worth ~= nil and worth(snap, facts, action) or nil
        }
    end)
end

function M.run(root, snap, facts)
    return run_tree(root, snap, facts)
end

function M.task(name)
    return leaf(name, true)
end

function M.must(name)
    return leaf(name, false)
end

return M
