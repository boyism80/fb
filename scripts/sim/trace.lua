local M = {}

function M.nodes(main_tree, choose, snap, bound)
    local log, restore, seen = {}, {}, {}
    local function wrap(node)
        if type(node) ~= "table" or seen[node] then
            return
        end
        seen[node] = true
        if type(node.run) == "function" then
            local run = node.run
            restore[#restore + 1] = function()
                node.run = run
            end
            node.run = function(...)
                local result = run(...)
                local action = result ~= nil and result.action or nil
                log[#log + 1] = tostring(node.name) .. "=" .. (result == nil and "nil" or
                    (tostring(action and action.kind) .. ":" .. tostring(action and (action.key or action.item)) .. ":" ..
                        tostring(result.worth)))
                return result
            end
        end
        if type(node.test) == "function" then
            local test = node.test
            restore[#restore + 1] = function()
                node.test = test
            end
            node.test = function(...)
                local result = test(...)
                log[#log + 1] = tostring(node.name) .. "?" .. tostring(result)
                return result
            end
        end
        wrap(node.child)
        wrap(node.inner)
        for _, child in ipairs(node.children or {}) do
            wrap(child)
        end
    end
    wrap(main_tree)
    local ok, result = pcall(choose, snap, bound)
    for _, undo in ipairs(restore) do
        undo()
    end
    if ok == false then
        error(result, 0)
    end
    return log, result
end

function M.described(decision)
    if decision == nil then
        return "none"
    end
    local dest = decision.dest or {}
    return table.concat(
    { tostring(decision.task), tostring(decision.kind), tostring(decision.key), tostring(decision.slot),
        tostring(decision.item), tostring(decision.oid), tostring(dest.map), tostring(dest.x), tostring(dest.y),
        tostring(dest.range), table.concat(dest.maps or {}, ",") }, "|")
end

function M.compare(left, right)
    local lines = {}
    for index = 1, math.max(#left, #right) do
        lines[#lines + 1] = string.format("%s %-72s | %s", left[index] == right[index] and "  " or "!!",
            tostring(left[index]), tostring(right[index]))
    end
    return lines
end

return M
