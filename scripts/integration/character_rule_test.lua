local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local MSG_MULTIPLIER_RANGE = "배율은 0~100 사이의 숫자여야 합니다."

local function fail(message)
    log("fatal", "Character rule test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

test_suite {
    name      = "Character Rule Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if run_step(bot, "verify_none_exp_cap", "none_exp_cap") == false then
                return fail("a class NONE character above the exp cap gained exp")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            if run_step(bot, "verify_potion_refund", "potion_refund", "갈색시약") == false then
                return fail("a potion that failed for lack of mana was consumed")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            for _, command in ipairs({ "/경험치배율 1000", "/드롭률배율 -1" }) do
                local packet = bot:request(resp.message, protocol.chat(false, command), function(p)
                    return p.text ~= nil and p.text:find(MSG_MULTIPLIER_RANGE, 1, true) ~= nil
                end)
                if packet == nil or packet == false then
                    return fail(command .. " was not refused as out of range")
                end
            end
            return true
        end,
    },
}
