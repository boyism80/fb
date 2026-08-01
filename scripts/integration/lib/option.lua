local resp     = require("integration.response")
local protocol = require("integration.protocol")

local M = {}

-- update_option toggles; keep requesting until the STATE message matches desired.
function M.set_option(bot, option_name, enabled, label)
    label = label or option_name
    for _ = 1, 2 do
        local packet = bot:request(
            resp.message,
            protocol.update_option(option_name, false),
            function(p)
                return p.type == "STATE" and p.text:find(label, 1, true) ~= nil
            end)
        if packet == false or packet == nil then
            return false
        end

        local is_on = packet.text:find(": ON", 1, true) ~= nil
        if is_on == enabled then
            return true
        end
    end
    return false
end

-- Character-vs-character harm (attack / damage / debuff) is blocked while PK_PROTECT is ON.
function M.disable_pk_protect(bot)
    return M.set_option(bot, "PK_PROTECT", false, "PK보호")
end

return M
