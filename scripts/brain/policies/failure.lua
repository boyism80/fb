local M = {}

local PERMANENT = { DENIED = true, REFUSED = true, FULL = true }

function M.remaining(snap, task)
    local record = snap.record.failures[task]
    if record == nil then
        return 0
    elseif PERMANENT[record.reason] then
        return (record.map ~= snap.world.map or record.level ~= snap.self.level) and 0 or nil
    else
        return math.max(0, record.duration * 2 ^ (record.failures - 1) - record.age)
    end
end

function M.allowed(snap, task)
    return M.remaining(snap, task) == 0
end

return M
