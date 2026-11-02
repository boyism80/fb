local data = require("brain.data.combat")

local M = {}

function M.visible(snap, spot)
    return spot.map == snap.world.map and math.abs(spot.x - snap.world.x) <= data.sight.x and
        math.abs(spot.y - snap.world.y) <= data.sight.y
end

function M.approach(snap, query, names)
    local maps    = {}
    local nearest = nil
    local best    = nil
    for _, spot in ipairs(query.npcs(names)) do
        table.insert(maps, spot.map)
        local cost = math.abs(spot.x - snap.world.x) + math.abs(spot.y - snap.world.y)
        if spot.map == snap.world.map and (best == nil or cost < best) then
            nearest = spot
            best    = cost
        end
    end
    if nearest ~= nil and M.visible(snap, nearest) then
        return nearest, nil
    elseif nearest ~= nil then
        local range = math.min(data.sight.x, data.sight.y)
        return nil, { kind = ACTION.MOVE, dest = { map = nearest.map, x = nearest.x, y = nearest.y, range = range } }
    elseif #maps > 0 then
        return nil, { kind = ACTION.MOVE, dest = { maps = maps } }
    end
    return nil, nil
end

return M
