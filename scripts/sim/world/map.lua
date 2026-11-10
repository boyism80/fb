local M = {}

local SOUTH, NORTH, WEST, EAST = 0x01, 0x02, 0x04, 0x08
local SCREEN = { width = 17, height = 15 }

local Map = {}
Map.__index = Map

function M.new(id, grid, enums)
    local map = setmetatable({
        id = id,
        width = grid.width,
        height = grid.height,
        blocked = grid.blocked,
        collisions = grid.collisions,
        option = grid.option or 0,
        spawns = grid.spawns or {},
        objects = {},
        occupied = {},
        enums = enums,
    }, Map)
    return map
end

function Map:index(x, y)
    return y * self.width + x + 1
end

function Map:inside(x, y)
    return x >= 0 and y >= 0 and x < self.width and y < self.height
end

function Map:open(x, y)
    return self:inside(x, y) and self.blocked:byte(self:index(x, y)) == 0
end

function Map:step(x, y, direction)
    local DIRECTION = self.enums.DIRECTION
    if direction == DIRECTION.BOTTOM then
        return x, y + 1, SOUTH, NORTH
    elseif direction == DIRECTION.TOP then
        return x, y - 1, NORTH, SOUTH
    elseif direction == DIRECTION.LEFT then
        return x - 1, y, WEST, EAST
    end
    return x + 1, y, EAST, WEST
end

function Map:enter(object)
    self.objects[object] = true
    local key = self:index(object.x, object.y)
    self.occupied[key] = (self.occupied[key] or 0) + 1
end

function Map:leave(object)
    if self.objects[object] == nil then
        return
    end
    self.objects[object] = nil
    local key = self:index(object.x, object.y)
    self.occupied[key] = self.occupied[key] - 1
end

function Map:shift(object, x, y)
    self:leave(object)
    object.x, object.y = x, y
    self:enter(object)
end

function Map:at(x, y, kind)
    if self.occupied[self:index(x, y)] == nil or self.occupied[self:index(x, y)] == 0 then
        return {}
    end
    local found = {}
    for object in pairs(self.objects) do
        if object.x == x and object.y == y and (kind == nil or object:is(kind)) then
            table.insert(found, object)
        end
    end
    return found
end

function Map:vacant(x, y, mover)
    if self:open(x, y) == false then
        return false
    end
    local count = self.occupied[self:index(x, y)] or 0
    if mover ~= nil and mover.x == x and mover.y == y and self.objects[mover] then
        count = count - 1
    end
    return count == 0
end

function Map:passable(x, y, direction, mover, walls)
    local tx, ty, leave, enter = self:step(x, y, direction)
    if (walls and self:open(tx, ty) or walls == nil and self:vacant(tx, ty, mover)) == false then
        return false
    end
    if (self.collisions:byte(self:index(x, y)) & leave) ~= 0 then
        return false
    end
    return (self.collisions:byte(self:index(tx, ty)) & enter) == 0, tx, ty
end

local function axis(position, size, screen)
    local half = screen // 2
    if position <= half then
        return 0, screen
    elseif position >= size - half then
        return math.max(0, size - screen - 1), math.max(0, size - 1)
    end
    return math.max(0, position - half - 1), math.max(0, position + half + 1)
end

function Map:sight(from, to)
    local x0, x1 = axis(from.x, self.width, SCREEN.width)
    local y0, y1 = axis(from.y, self.height, SCREEN.height)
    return to.x >= x0 and to.x <= x1 and to.y >= y0 and to.y <= y1
end

function Map:route(mover, goal, walls, barred)
    local DIRECTION = self.enums.DIRECTION
    local start     = self:index(mover.x, mover.y)
    if goal(mover.x, mover.y) then
        return {}
    end
    local parents = { [start] = false }
    local queue   = { { mover.x, mover.y } }
    local head    = 1
    while head <= #queue do
        local x, y = queue[head][1], queue[head][2]
        head = head + 1
        for _, direction in ipairs({ DIRECTION.TOP, DIRECTION.RIGHT, DIRECTION.BOTTOM, DIRECTION.LEFT }) do
            local allowed, tx, ty = self:passable(x, y, direction, mover, walls)
            if allowed and (barred == nil or barred[self:index(tx, ty)] == nil) then
                local key = self:index(tx, ty)
                if parents[key] == nil then
                    parents[key] = { from = self:index(x, y), direction = direction }
                    if goal(tx, ty) then
                        local path = {}
                        while key ~= start do
                            table.insert(path, 1, parents[key].direction)
                            key = parents[key].from
                        end
                        return path
                    end
                    table.insert(queue, { tx, ty })
                end
            end
        end
    end
    return nil
end

return M
