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

            local steps      = {}
            local promotions = model.promotions()
            for i, promotion in ipairs(promotions) do
                local bot = ctx:bot(i - 1)
                bot:change_class(promotion.name)
                bot:chat("/레벨바꾸기 99")
                counts[promotion.cls] = (counts[promotion.cls] or 0) + 1
                steps[bot:name()]     = promotion.step
            end

            -- change_class is fire-and-forget (/직업바꾸기 has no waitable class-name response).
            ctx:sleep(2000)

            local matches = function(packet)
                if packet == nil or packet == false or #packet.users < bot_count then
                    return false
                end

                local missing = {}
                for name in pairs(names) do
                    missing[name] = true
                end
                local remaining = {}
                for cls, count in pairs(counts) do
                    remaining[cls] = count
                end

                for _, user in ipairs(packet.users) do
                    if missing[user.name] then
                        missing[user.name] = nil
                        remaining[user.cls] = (remaining[user.cls] or 0) - 1
                    end
                    if steps[user.name] ~= nil and steps[user.name] ~= user.promotion then
                        return false
                    end
                end

                for _, count in pairs(remaining) do
                    if count ~= 0 then
                        return false
                    end
                end
                return next(missing) == nil
            end

            for _ = 1, 7 do
                local packet = ctx:bot(0):request(
                    resp.user_list,
                    protocol.user_list(),
                    function()
                        return true
                    end)
                if matches(packet) then
                    return true
                end
                ctx:sleep(2000)
            end
            return false
        end,
    },
}
