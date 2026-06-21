local resp     = require("integration.response")
local protocol = require("integration.protocol")

local MESSAGE_EXCEPTION_INVENTORY_OVERFLOW = "소지품이 꽉 찼습니다."

function run_overflow(ctx)
    local bot = ctx:bot(0)

    bot:fill_inventory("목도")
    log("debug", "Scenario 3-1: Full inventory")

    if bot:equip(0) == false then
        return false
    end
    log("debug", "Scenario 3-1: Equipped 1 목도 and created 1 more")

    bot:create_item("목도", 1)
    log("debug", "Scenario 3-1: Full inventory again")

    local packet = bot:request(
        resp.message,
        protocol.item_inactive("WEAPON"),
        function(r)
            return r.type == "STATE"
        end)

    if packet == nil or string.find(packet.text, MESSAGE_EXCEPTION_INVENTORY_OVERFLOW, 1, true) == nil then
        return false
    end

    return true
end

return { run_overflow = run_overflow }
