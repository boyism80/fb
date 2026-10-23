local protocol = require("integration.protocol")
local resp     = require("integration.response")

local M = {}

local DEFAULT_TIMEOUT_MS   = 5000
local SETTLE_DEADLINE_MS   = 40000
local SETTLE_TIMER_MESSAGE = "정산 타이머"

local LOCAL_SETTLE  = { interval = 1000, margin = 2000, poll = 500 }
local REMOTE_SETTLE = { interval = 30000, margin = 5000, poll = 5000 }

local function settle_profile()
    if localhost() then
        return LOCAL_SETTLE
    else
        return REMOTE_SETTLE
    end
end

local function set_settle_timer(bot, state)
    local packet = bot:request(resp.message, protocol.chat(false, "/정산타이머 " .. state), function(p)
        return p.text ~= nil and p.text:find(SETTLE_TIMER_MESSAGE, 1, true) ~= nil
    end)
    return packet ~= nil and packet ~= false
end

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

function M.settle_interval()
    return settle_profile().interval
end

function M.settle_tick(ctx, extra_ms)
    local profile = settle_profile()
    ctx:sleep(profile.interval + profile.margin + (extra_ms or 0))
end

function M.settle(ctx, check, on_retry)
    local profile  = settle_profile()
    local deadline = profile.interval + SETTLE_DEADLINE_MS
    local waited   = 0
    local attempt  = 0
    while check() ~= true do
        if waited >= deadline then
            return false
        end
        attempt = attempt + 1
        if on_retry ~= nil then
            on_retry(attempt)
        end
        ctx:sleep(profile.poll)
        waited = waited + profile.poll
    end
    return true
end

function M.pause_settle(bot)
    return set_settle_timer(bot, "off")
end

function M.resume_settle(bot)
    return set_settle_timer(bot, "on")
end

return M
