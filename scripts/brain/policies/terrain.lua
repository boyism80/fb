local memo = require("brain.policies.memo")

local M = {}

local grids   = {}
local detours = {}

function M.floor(source, map, x, y)
    local grid = grids[map]
    if grid == nil then
        grid       = source:grid(map) or false
        grids[map] = grid
    end
    if grid == false or x < 0 or y < 0 or x >= grid.width or y >= grid.height then
        return false
    end
    return string.byte(grid.blocked, y * grid.width + x + 1) == 0
end

function M.detour(source, map, from, to, avoided)
    local spots = string.format("%d,%d|%d,%d", from.x, from.y, avoided.x, avoided.y)
    return memo.get(detours, tostring(map), to.x .. "," .. to.y, spots, function()
        local goal   = { { x = to.x, y = to.y, range = 0 } }
        local free   = source:steps(map, from.x, from.y, goal)[1]
        local around = source:steps(map, from.x, from.y, goal, { { x = avoided.x, y = avoided.y } })[1]
        if type(free) ~= "number" then
            return 0
        end
        return type(around) == "number" and around - free or math.huge
    end)
end

return M
