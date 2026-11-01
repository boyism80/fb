local expect   = require("integration.lib.expect")
local protocol = require("integration.protocol")
local resp     = require("integration.response")

local M = {}

local ITEM    = "도토리"
local COMMAND = "/아이템생성 도토리 1"
local TIMEOUT = 5000

function M.run(ctx, index)
    local bot = ctx:bot(index)
    bot:clear_inventory()

    local mind = brain.new()
    bot:brain(mind, { seed = 1, halt = { task = "TUTORIAL", step = 0 } })
    bot:chat(COMMAND)
    local settled = bot:request(resp.internal_info, protocol.self_info(), function() return true end, TIMEOUT) ~= false
    local blocked = settled and bot:has_item_by_name(ITEM) == false
    bot:brain(nil)

    bot:chat(COMMAND)
    local allowed = expect.within(ctx, TIMEOUT, function()
        return bot:has_item_by_name(ITEM)
    end)
    bot:clear_inventory()

    if blocked and allowed then
        log("debug", "[brain:guard:command] SUCCESS")
    else
        log("fatal",
            string.format("[brain:guard:command] FAILED blocked=%s allowed=%s", tostring(blocked), tostring(allowed)))
    end
    return blocked and allowed
end

return M
