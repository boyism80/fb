local script_step = require("integration.lib.script_step")

local function fail(message)
    log("fatal", "Buff concast test: " .. message)
    return false
end

test_suite {
    name      = "Buff Concast Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot:level(99)

            local restored_after_death = 0
            for _, order in ipairs({ "kill_first", "heal_first" }) do
                local ok, text = script_step.run_script(bot, "test/integration", "concast_after_death",
                    "concast_" .. order, order)
                if ok ~= true then
                    return fail(string.format("%s: the character did not die from the concast damage (%s)", order,
                        tostring(text)))
                end

                local mp = tonumber(tostring(text):match("mp=(%d+)"))
                if mp == nil then
                    return fail(string.format("%s: no mp in the report (%s)", order, tostring(text)))
                end
                if mp > 0 then
                    restored_after_death = restored_after_death + 1
                end
            end

            if restored_after_death == 2 then
                return fail("운기 restored mp in the death tick whichever buff ran first")
            end
            return true
        end,
    },
}
