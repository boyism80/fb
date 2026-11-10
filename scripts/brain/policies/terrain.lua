local M = {}

local grids = {}

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

return M
