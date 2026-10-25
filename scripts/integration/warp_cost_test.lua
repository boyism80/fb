local lib      = require("integration.lib")
local qd       = require("integration.lib.quest_dialog")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local check = qd.check

local function fail(fmt, ...)
    log("fatal", "warp_cost: " .. string.format(fmt, ...))
    return false
end

local function use_warp_item(ctx, item_name, hosted_func, hosted_step, ...)
    local bot = ctx:bot(0)
    bot:clear_inventory()
    if check(bot, "make_items", "make_" .. hosted_step, item_name, 1, 0) == false then
        return fail("%s: could not create the item", item_name)
    end

    local hosted = check(bot, hosted_func, hosted_step, ...)
    local map_before = bot:map()
    if hosted then
        local packet = bot:request(resp.map_config, protocol.item_active(0), function(p)
            return p.id ~= map_before
        end)
        if type(packet) ~= "table" then
            return fail("%s: no warp after using the item", item_name)
        end
        if check(bot, "verify_items", "consumed_" .. hosted_step, item_name .. "=0") == false then
            return fail("%s: the warp did not consume the item", item_name)
        end
    else
        local packet = bot:request(resp.message, protocol.item_active(0), function(p)
            return p.type == "STATE"
        end)
        if type(packet) ~= "table" then
            return fail("%s: the refused warp was not reported", item_name)
        end
        if bot:map() ~= map_before then
            return fail("%s: moved although the destination is not hosted here", item_name)
        end
        if check(bot, "verify_items", "kept_" .. hosted_step, item_name .. "=1") == false then
            return fail("%s: a refused warp consumed the item", item_name)
        end
    end

    lib.formation.arrange_in_line(ctx)
    return true
end

test_suite {
    name      = "Warp Cost Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        ctx:bot(0):clear_inventory()
        lib.formation.arrange_in_line(ctx)
    end,

    scenarios = {
        function(ctx)
            return use_warp_item(ctx, "장안성비서", "verify_map_active", "jangan_hosted", "장안성주막")
        end,

        function(ctx)
            return use_warp_item(ctx, "노란비서", "verify_return_map_active", "return_hosted")
        end,
    },
}
