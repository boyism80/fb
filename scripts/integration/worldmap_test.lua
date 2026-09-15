local resp     = require("integration.response")
local protocol = require("integration.protocol")
local model    = require("integration.model")
local bot_diag = require("integration.lib.bot_diag")

test_suite {
    name      = "Worldmap Test",
    bot_count = 1,

    should_skip = function(_ctx)
        if localhost() then
            log("debug", "Worldmap test skipped on local host (requires server transfer)")
            return true
        end
        return false
    end,

    scenarios = {
        function(ctx)
            log("debug", "Starting worldmap test")
            local bot = ctx:bot(0)
            bot_diag.dump(bot, "worldmap:start")

            -- Open the world-map UI with an admin command. Going to 국내성진입로 and
            -- walking the warp tile needs a cross-host /맵이동 that intermittently
            -- never sends S2C transfer; bot transfer timeouts are not catchable via
            -- pcall (co_builder reject), so that setup hop cannot be retried safely.
            -- /월드맵 still exercises show_world_map → map_worlds (same listener path).
            local worlds = bot:request(
                resp.map_worlds,
                protocol.chat(false, "/월드맵 국내성"),
                function()
                    return true
                end)
            if worlds == false or worlds == nil then
                log("fatal", "Worldmap test: map_worlds via /월드맵 failed")
                bot_diag.dump(bot, "worldmap:map_worlds_fail")
                return false
            end
            log("debug", string.format(
                "Worldmap test: map_worlds ok key=%s index=%s world_count=%s",
                tostring(worlds.key), tostring(worlds.index), tostring(worlds.world_count)))

            -- Navigate from 국내성 (index 1) to 부여성 (index 0) and transfer there.
            local ok, err = pcall(function()
                bot:transfer(protocol.map_world(0, 1, 0))
            end)
            if ok == false then
                log("fatal", "Worldmap test: map_world transfer failed: " .. tostring(err))
                bot_diag.dump(bot, "worldmap:after_world_transfer_fail")
                return false
            end
            bot = ctx:bot(0)
            bot_diag.dump(bot, "worldmap:after_world_transfer")

            local dest = model.world_destination(0, 0)
            if bot:map() ~= dest.map then
                log("fatal", string.format(
                    "Worldmap test: dest map mismatch got=%s expected=%s",
                    tostring(bot:map()),
                    tostring(dest.map)))
                bot_diag.dump(bot, "worldmap:dest_map_mismatch")
                return false
            end

            local position = bot:position()
            if position[1] ~= dest.position[1] or position[2] ~= dest.position[2] then
                log("fatal", string.format(
                    "Worldmap test: dest pos mismatch got=(%s,%s) expected=(%s,%s)",
                    tostring(position[1]), tostring(position[2]),
                    tostring(dest.position[1]), tostring(dest.position[2])))
                bot_diag.dump(bot, "worldmap:dest_pos_mismatch")
                return false
            end

            return true
        end,
    },
}
