local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local KINDS = { "paralysis", "cover" }

local function fail(message)
    log("fatal", "Worldmap restraint test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

test_suite {
    name      = "Worldmap Restraint Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        for _, kind in ipairs(KINDS) do
            run_step(bot, "set_restraint", "worldmap_restraint_reset_" .. kind, kind, "false")
        end
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            for _, kind in ipairs(KINDS) do
                local worlds = bot:request(
                    resp.map_worlds,
                    protocol.chat(false, "/월드맵 국내성"),
                    function()
                        return true
                    end)
                if worlds == nil or worlds == false then
                    return fail(kind .. ": /월드맵 did not open the world map")
                end

                if run_step(bot, "set_restraint", "worldmap_restraint_on_" .. kind, kind, "true") == false then
                    return fail(kind .. ": could not set the restraint")
                end

                local map = bot:map()
                local position = bot:request(
                    resp.position,
                    protocol.map_world(0, 1, 0),
                    function()
                        return true
                    end)
                if position == nil or position == false then
                    return fail(kind .. ": the world map move was not refused with a position refresh")
                end
                if bot:map() ~= map then
                    return fail(string.format("%s: the bot left map %s for %s", kind, tostring(map), tostring(bot:map())))
                end

                if run_step(bot, "set_restraint", "worldmap_restraint_off_" .. kind, kind, "false") == false then
                    return fail(kind .. ": could not clear the restraint")
                end
            end
            return true
        end,
    },
}
