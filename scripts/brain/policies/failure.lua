local data = require("brain.data.combat")

local M = {}

local PERMANENT = { DENIED = true, REFUSED = true, FULL = true }

function M.remaining(snap, task)
    local record = snap.record.failures[task]
    if record == nil or record.level ~= snap.self.level then
        return 0
    elseif PERMANENT[record.reason] or (task == "LOOT" and record.reason == "LOOP") then
        return record.map ~= snap.world.map and 0 or nil
    else
        local wait = record.duration * 2 ^ (record.failures - 1)
        return math.max(0, (record.reason == "BLOCKED" and math.min(wait, data.rerank) or wait) - record.age)
    end
end

function M.allowed(snap, task)
    return M.remaining(snap, task) == 0
end

return M
