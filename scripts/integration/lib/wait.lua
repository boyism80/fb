local M = {}

local DEFAULT_TIMEOUT_MS = 5000

-- Waits until predicate() holds, re-checking on every `response` packet the bot receives.
-- The bot applies a packet to its state before validators run, and the first check plus the
-- hook registration run on the suite thread without yielding, so a packet cannot slip between them.
function M.state(bot, response, predicate, timeout)
    if predicate() then
        return true
    end

    local packet = bot:request(response, nil, function()
        return predicate()
    end, timeout or DEFAULT_TIMEOUT_MS)
    return packet ~= false
end

return M
