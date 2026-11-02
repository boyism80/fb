local M = {}

function M.wanted(snap)
    return snap.world.dialog
end

function M.propose()
    return { kind = ACTION.TALK }
end

return M
