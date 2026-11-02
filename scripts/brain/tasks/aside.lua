local M = {}

function M.wanted(snap)
    return snap.self.ghost == false and snap.world.aside ~= nil
end

function M.propose(snap)
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = snap.world.aside.x, y = snap.world.aside.y, range = 0 } }
end

return M
