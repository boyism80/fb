local resp     = require("integration.response")
local protocol = require("integration.protocol")
local model    = require("integration.model")

local MESSAGE_NO_RECIPE   = "조합할 수 없습니다."
local MESSAGE_MIX_SUCCESS = "성공하였습니다."

local DEFAULT_INTERVAL = 100

function run(ctx, index)
    local bot       = ctx:bot(index)
    local bot_count = ctx:bot_count()

    for recipe_index, recipe in ipairs(model.recipes()) do
        if (recipe_index - 1) % bot_count == index then
            bot:clear_inventory()
            ctx:sleep(DEFAULT_INTERVAL)

            local expected_success = {}
            local expected_failed  = {}
            for _, item in ipairs(recipe.success) do
                table.insert(expected_success, item.name)
            end
            for _, item in ipairs(recipe.failed) do
                table.insert(expected_failed, item.name)
            end

            log("debug", string.format(
                "Scenario 3-1: Expected items: %s or %s",
                table.concat(expected_success, ", "),
                table.concat(expected_failed, ", ")))

            local slots = {}
            local slot  = 0
            for _, source in ipairs(recipe.source) do
                local remain = source.count
                while remain > 0 do
                    local count = math.min(source.capacity, remain)
                    remain = remain - count
                    bot:create_item(source.name, count)
                    table.insert(slots, slot)
                    slot = slot + 1
                end
            end

            local packet = bot:request(
                resp.message,
                protocol.item_combine(slots),
                function(r)
                    return r.type == "STATE"
                end)

            if packet == nil then
                return false
            end

            if string.find(packet.text, MESSAGE_NO_RECIPE, 1, true) ~= nil then
                log("debug", "Scenario 3-1: No recipe found")
                return false
            end

            local success = string.find(packet.text, MESSAGE_MIX_SUCCESS, 1, true) ~= nil
            if success then
                for _, item in ipairs(recipe.success) do
                    if bot:has_item_by_name(item.name) == false then
                        return false
                    end
                    if bot:item_count(item.name) ~= item.count then
                        return false
                    end
                end
            else
                for _, item in ipairs(recipe.failed) do
                    if bot:has_item_by_name(item.name) == false then
                        return false
                    end
                end
            end

            log("debug", string.format("combine result : %s", success and "success" or "failed"))
        end
    end

    return true
end

function run_failure(ctx)
    local bot = ctx:bot(0)

    bot:create_item("목도", 1)
    bot:create_item("목검", 1)
    bot:create_item("사두목도", 1)
    bot:create_item("사두목검", 1)
    bot:create_item("뢰진도", 1)

    log("debug", "Scenario 3-2: Try to combine items with no recipe")
    bot:request(
        resp.message,
        protocol.item_combine({ 0, 1, 2, 3, 4 }),
        function(r)
            return r.type == "STATE"
                and string.find(r.text, MESSAGE_NO_RECIPE, 1, true) ~= nil
        end)

    log("debug", "Scenario 3-2: Successfully combined items with no recipe")
    return true
end

return {
    run          = run,
    run_failure  = run_failure,
}
