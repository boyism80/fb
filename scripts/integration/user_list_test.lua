local lib      = require("integration.lib")
local model    = require("integration.model")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

test_suite {
    name      = "User List Test",
    bot_count = 21,

    on_initialize = function(ctx)
        log("debug", "User list test initialized")
        lib.formation.arrange_in_grid(ctx, "낙랑의방", 5, 5, 15, 13)
    end,

    scenarios = {
        function(ctx)
            log("debug", "Starting user list test")
            local bot_count = ctx:bot_count()
            local names     = {}
            local counts    = {}

            for i = 0, bot_count - 1 do
                names[ctx:bot(i):name()] = true
            end

            local promotions = model.promotions()
            for i, promotion in ipairs(promotions) do
                local bot = ctx:bot(i - 1)
                bot:change_class(promotion.name)
                counts[promotion.cls] = (counts[promotion.cls] or 0) + 1
            end

            local packet = ctx:bot(0):request(
                resp.user_list,
                protocol.user_list(),
                function()
                    return true
                end)

            if #packet.users < bot_count then
                return false
            end

            for _, user in ipairs(packet.users) do
                if names[user.name] then
                    names[user.name] = nil
                    counts[user.cls] = counts[user.cls] - 1
                end
            end

            for _, count in pairs(counts) do
                if count ~= 0 then
                    return false
                end
            end

            return true
        end,
    },
}
