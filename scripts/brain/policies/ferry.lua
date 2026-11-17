local data = require("brain.data.ferry")

local M = {}

local function level_of(origin, fallback)
    if origin ~= nil and origin.level ~= nil then
        return origin.level
    end
    return fallback or 0
end

local function copy_set(set)
    local found = {}
    for map, open in pairs(set or {}) do
        if open then
            found[map] = true
        end
    end
    return found
end

local function merge(into, from)
    for map, open in pairs(from or {}) do
        if open then
            into[map] = true
        end
    end
    return into
end

local function hop(route, leg, from, dest, after)
    return {
        map = from.map,
        x = from.x,
        y = from.y,
        dest = dest.map,
        ax = after.x,
        ay = after.y,
        world = false,
        ferry = true,
        route = route.id,
        leg = leg,
    }
end

local function append(path, hops)
    for _, step in ipairs(hops or {}) do
        table.insert(path, step)
    end
end

local function compose(to_board, route, from_land)
    local path = {}
    append(path, to_board.path)
    table.insert(path, hop(route, "board", route.board, route.cabin, route.cabin))
    table.insert(path, hop(route, "land", route.cabin, route.land, route.land))
    append(path, from_land.path)
    return {
        dest = from_land.dest,
        cost = (to_board.cost or 0) + route.cost + (from_land.cost or 0),
        hops = #path,
        path = path,
        x = from_land.x,
        y = from_land.y,
        ferry = route.id,
    }
end

function M.routes()
    return data.routes
end

function M.of(id)
    for _, route in ipairs(data.routes) do
        if route.id == id then
            return route
        end
    end
end

function M.widen(raw_reach, perilous, shunned, origin)
    local base = copy_set(raw_reach(perilous, shunned, origin))
    local rank = level_of(origin)
    for _, route in ipairs(data.routes) do
        if rank >= route.min_level then
            local pier = base[route.board.map] == true
            local cabin = origin ~= nil and origin.map == route.cabin.map
            if pier or cabin then
                base[route.cabin.map] = true
                merge(base, raw_reach(perilous, shunned, {
                    map = route.land.map,
                    x = route.land.x,
                    y = route.land.y,
                    level = rank,
                }))
            end
        end
    end
    return base
end

function M.bridge(raw_route, dest, perilous, shunned, origin)
    local direct = raw_route(dest, perilous, shunned, origin)
    local best = direct
    local rank = level_of(origin)
    for _, route in ipairs(data.routes) do
        if rank >= route.min_level then
            local to_board = raw_route({
                map = route.board.map,
                x = route.board.x,
                y = route.board.y,
            }, perilous, shunned, origin)
            if to_board ~= nil then
                local from_land = raw_route(dest, perilous, shunned, {
                    map = route.land.map,
                    x = route.land.x,
                    y = route.land.y,
                    level = rank,
                })
                if from_land ~= nil then
                    local via = compose(to_board, route, from_land)
                    if best == nil or via.cost < best.cost then
                        best = via
                    end
                end
            end
        end
    end
    return best
end

function M.needed(route_path)
    for _, step in ipairs(route_path or {}) do
        if step.ferry then
            return M.of(step.route), step.leg
        end
    end
end

return M
