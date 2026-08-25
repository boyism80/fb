-- Period events from event.xlsx (string id PK).

local M = {}

function M.is(id)
    if id == nil or id == '' then
        return false
    end
    return event_is(id) == true
end

return M
