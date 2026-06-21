local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local DEFAULT_INTERVAL = 100

test_suite {
    name      = "Attack Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Attack test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        ctx:bot(0):clear_all_drop_items()
    end,

    scenarios = {
        function(ctx)
            local attacker = ctx:bot(0)
            local died       = false
            local target_oid = nil

            log("debug", "Enhanced attack test: Setting up bot for combat")

            local spell_index = attacker:learn_spell("유인")
            log("debug", "Spell index: " .. tostring(spell_index))
            attacker:set_max_hp_mp(100000, 100000)
            attacker:level(77)
            attacker:create_item("월아검", 1)

            if attacker:equip(0) == false then
                return false
            end

            local pos      = attacker:position()
            local monster  = attacker:spawn_monster("초급유령", pos[1], pos[2] + 1)
            target_oid     = monster.oid
            log("debug", string.format(
                "Enhanced attack test: Bot setup complete, monster spawned at (%d, %d)",
                pos[1], pos[2] + 1))

            ctx:hook("die", function(_, _, packet)
                if target_oid ~= nil and packet.oid == target_oid then
                    died = true
                end
            end)

            log("debug", "Click to ghost")
            attacker:request(
                resp.message,
                protocol.click(monster.oid),
                function(packet)
                    return packet.text == "초급유령"
                end)

            log("debug", "Cast spell")
            attacker:request(
                resp.action,
                protocol.spell_cast("TARGET", spell_index, "", monster.oid, monster.position),
                function(packet)
                    return packet.oid == attacker:oid()
                end)

            while died == false do
                attacker:request(
                    resp.action,
                    protocol.attack(),
                    function(packet)
                        return packet.oid == attacker:oid() and packet.value == "ATTACK"
                    end)
                ctx:sleep(DEFAULT_INTERVAL)
            end

            ctx:unhook("die")
            return true
        end,

        function(ctx)
            local attacker = ctx:bot(0)
            local target   = ctx:bot(1)

            log("debug", "Enhanced attack test: PvP scenario started")

            target:set_max_hp_mp(100000, 100000)
            attacker:direction("RIGHT")

            while target:state() ~= "GHOST" do
                attacker:request(
                    resp.action,
                    protocol.attack(),
                    function(packet)
                        return packet.oid == attacker:oid() and packet.value == "ATTACK"
                    end)
                ctx:sleep(DEFAULT_INTERVAL)
            end

            return true
        end,
    },
}
