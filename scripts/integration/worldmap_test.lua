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

            local ok, err = pcall(function()
                bot:transfer(protocol.chat(false, "/맵이동 국내성진입로 8 1"))
            end)
            if ok == false then
                log("fatal", "Worldmap test: transfer to 국내성진입로 failed: " .. tostring(err))
                bot_diag.dump(bot, "worldmap:after_entrance_transfer_fail")
                return false
            end
            bot = ctx:bot(0)
            bot_diag.dump(bot, "worldmap:after_entrance_transfer")

            local pos = bot:position()
            local worlds = bot:request(
                resp.map_worlds,
                protocol.move("TOP", bot:oid(), pos),
                function()
                    return true
                end)
            if worlds == false or worlds == nil then
                log("fatal", "Worldmap test: map_worlds request failed")
                bot_diag.dump(bot, "worldmap:map_worlds_fail")
                return false
            end
            log("debug", string.format("Worldmap test: map_worlds ok oid=%s pos=(%s,%s)",
                tostring(bot:oid()), tostring(pos[1]), tostring(pos[2])))

            ok, err = pcall(function()
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
