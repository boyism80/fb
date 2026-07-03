local formation = require("integration.lib.formation")
local resp      = require("integration.response")
local protocol  = require("integration.protocol")

local MESSAGE_POISON_CLEAR = "중독 해제"
local GEAR_CLEAR_WAIT_MS   = 5000

test_suite {
    name      = "Concast Test",
    bot_count = 4,

    on_initialize = function(ctx)
        log("debug", "Concast test initialized")
        formation.arrange_in_line(ctx)
    end,

    on_parallel_scenario_finished = function(ctx, id)
        local bot = ctx:bot(id)
        bot:clear_inventory()
        bot:remove_buffs()
        bot:setup_bot_stats(1000, 1000)
        ctx:sleep(500)
    end,

    scenarios = {
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local bot = ctx:bot(0)
                        log("debug", string.format("Concast[%s]: buff item test start item=보무의목걸이", bot:name()))

                        bot:setup_bot_stats(10000, 10000)
                        bot:remove_buffs()
                        bot:clear_inventory()
                        bot:create_item("보무의목걸이", 1)
                        bot:apply_item_condition("보무의목걸이")

                        local pending_buffs = {
                            ["자동무장"] = true,
                            ["자동보호"] = true,
                        }

                        local buffed = bot:request(
                            resp.spell_buff,
                            protocol.item_active(0),
                            function(packet)
                                if packet.name ~= nil and pending_buffs[packet.name] then
                                    pending_buffs[packet.name] = nil
                                end
                                return next(pending_buffs) == nil
                            end,
                            GEAR_CLEAR_WAIT_MS)

                        if buffed == false or buffed == nil then
                            log("fatal", string.format("Concast[%s]: expected buffs not applied", bot:name()))
                            return false
                        end

                        log("debug", string.format("Concast[%s]: buff item test passed", bot:name()))
                        return true
                    end,
                },
                [1] = {
                    function(ctx)
                        local bot = ctx:bot(1)
                        log("debug", string.format("Concast[%s]: buff item test start item=투명구두", bot:name()))

                        bot:setup_bot_stats(10000, 10000)
                        bot:remove_buffs()
                        bot:clear_inventory()
                        bot:create_item("투명구두", 1)
                        bot:apply_item_condition("투명구두")

                        local buffed = bot:request(
                            resp.spell_buff,
                            protocol.item_active(0),
                            function(packet)
                                return packet.name == "자동투명"
                            end,
                            GEAR_CLEAR_WAIT_MS)

                        if buffed == false or buffed == nil then
                            log("fatal", string.format("Concast[%s]: expected buff not applied", bot:name()))
                            return false
                        end

                        log("debug", string.format("Concast[%s]: buff item test passed", bot:name()))
                        return true
                    end,
                },
                [2] = {
                    function(ctx)
                        local bot = ctx:bot(2)

                        log("debug", string.format("Concast[%s]: poison clear test start", bot:name()))

                        bot:setup_bot_stats(10000, 10000)
                        bot:remove_buffs()
                        bot:clear_inventory()

                        local slot = bot:learn_spell("중독")
                        if slot < 0 then
                            log("fatal", string.format("Concast[%s]: failed to learn spell 중독", bot:name()))
                            return false
                        end

                        local applied = bot:request(
                            resp.spell_buff,
                            protocol.spell_cast("TARGET", slot, "", bot:oid(), bot:position()),
                            function(packet)
                                return packet.name == "중독"
                            end)

                        if applied == false or applied == nil then
                            log("fatal", string.format("Concast[%s]: failed to apply debuff 중독", bot:name()))
                            return false
                        end

                        bot:create_item("해독의귀걸이", 1)
                        bot:apply_item_condition("해독의귀걸이")

                        local poison_cleared = bot:request(
                            resp.message,
                            protocol.item_active(0),
                            function(packet)
                                return packet.type == "STATE"
                                    and packet.text:find(MESSAGE_POISON_CLEAR, 1, true) ~= nil
                            end,
                            GEAR_CLEAR_WAIT_MS)

                        if poison_cleared == false or poison_cleared == nil then
                            log("fatal", string.format("Concast[%s]: poison clear message not received", bot:name()))
                            return false
                        end

                        log("debug", string.format("Concast[%s]: poison clear test passed", bot:name()))
                        return true
                    end,
                },
                [3] = {
                    function(ctx)
                        local bot = ctx:bot(3)

                        log("debug", string.format("Concast[%s]: despair clear test start", bot:name()))

                        bot:setup_bot_stats(10000, 10000)
                        bot:remove_buffs()
                        bot:clear_inventory()

                        local slot = bot:learn_spell("절망")
                        if slot < 0 then
                            log("fatal", string.format("Concast[%s]: failed to learn spell 절망", bot:name()))
                            return false
                        end

                        local applied = bot:request(
                            resp.update_cc,
                            protocol.spell_cast("TARGET", slot, "", bot:oid(), bot:position()),
                            function(packet)
                                return packet.sight == true
                            end)

                        if applied == false or applied == nil then
                            log("fatal", string.format("Concast[%s]: failed to apply debuff 절망", bot:name()))
                            return false
                        end

                        bot:create_item("시력회복의귀걸이", 1)
                        bot:apply_item_condition("시력회복의귀걸이")

                        local despair_cleared = bot:request(
                            resp.update_cc,
                            protocol.item_active(0),
                            function(packet)
                                return packet.sight == false
                            end,
                            GEAR_CLEAR_WAIT_MS)

                        if despair_cleared == false or despair_cleared == nil then
                            log("fatal", string.format("Concast[%s]: crowd control sight not cleared", bot:name()))
                            return false
                        end

                        log("debug", string.format("Concast[%s]: despair clear test passed", bot:name()))
                        return true
                    end,
                },
            },
        },
    },
}
