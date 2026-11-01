local M = {}

local PERMANENT = { NO_ROUTE = true, DENIED = true, REFUSED = true, FULL = true }

function M.allowed(brain, bot, task)
    local record = brain:failure(bot, task)
    if record == nil then
        return true
    elseif PERMANENT[record.reason] then
        local seen = brain:observe(bot)
        return record.map ~= seen.map or record.level ~= seen.level
    else
        return record.age >= record.duration * 2 ^ (record.failures - 1)
    end
end

return M
