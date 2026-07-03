local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil
        and message:find("AS EXPECTED", 1, true) == nil
        and message:find("SUCCESS:", 1, true) == nil then
        level = "fatal"
    end
    log(level, message)
    bot:chat("=== " .. message .. " ===")
end

test_suite {
    name      = "Group Test",
    bot_count = 6,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "GROUP TEST INITIALIZED WITH " .. ctx:bot_count() .. " BOTS")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        ctx:bot(0):clear_all_drop_items()
        lib.group.cleanup(ctx)
    end,

    scenarios = {
        function(ctx)
            local caster = ctx:bot(0)
            progress(caster, "GROUP SCENARIO 1 TEST STARTED")

            progress(caster, "LEARNING SPELL AND SETTING UP MP")
            local spell_slot = caster:learn_spell("뢰진주")
            caster:mp(1000)

            progress(caster, "SPAWNING MONSTER FOR SOLO TEST")
            local pos = caster:position()
            local mob1 = caster:spawn_monster("다람쥐", pos[1], pos[2] + 1)
            local before_exp = caster:exp()

            caster:request(
                resp.update_internal,
                protocol.spell_cast("TARGET", spell_slot, "", mob1.oid, mob1.position),
                function(packet)
                    return packet.ch_exp ~= before_exp
                end)

            local solo_diff = caster:exp() - before_exp
            progress(caster, "SOLO ATTACK EXP GAIN: " .. tostring(solo_diff))

            before_exp = caster:exp()
            progress(caster, "FORMING GROUP FOR GROUP EXP TEST")
            lib.group.form(ctx)
            progress(caster, "GROUP FORMED SUCCESSFULLY")

            progress(caster, "SPAWNING MONSTER FOR GROUP TEST")
            caster:mp(1000)
            local pos2 = caster:position()
            local mob2 = caster:spawn_monster("다람쥐", pos2[1], pos2[2] + 1)
            ctx:sleep(skill.DEFAULT_INTERVAL)

            caster:request(
                resp.update_internal,
                protocol.spell_cast("TARGET", spell_slot, "", mob2.oid, mob2.position),
                function(packet)
                    return packet.ch_exp ~= before_exp
                end)

            local group_diff = caster:exp() - before_exp
            progress(caster, "GROUP ATTACK EXP GAIN: " .. tostring(group_diff))
            progress(caster, "COMPARING EXP GAINS - SOLO: " .. tostring(solo_diff) .. ", GROUP: " .. tostring(group_diff))

            if group_diff >= solo_diff then
                progress(caster, "GROUP SCENARIO 1 FAILED - GROUP EXP NOT REDUCED")
                return false
            end

            progress(caster, "GROUP EXP SHARING TEST PASSED")
            progress(caster, "GROUP SCENARIO 1 TEST COMPLETED SUCCESSFULLY")
            return true
        end,

        function(ctx)
            local leader = ctx:bot(0)
            progress(leader, "GROUP SCENARIO 2 TEST STARTED")

            local bots = {}
            for i = 0, 5 do
                bots[i] = ctx:bot(i)
            end

            progress(leader, "INVITING " .. bots[1]:name() .. " TO GROUP")
            if bots[0]:invite_group(bots[1]) == false then
                progress(leader, "FAILED TO INVITE " .. bots[1]:name() .. " TO GROUP")
                return false
            end
            progress(leader, "SUCCESSFULLY INVITED " .. bots[1]:name() .. " TO GROUP")

            progress(leader, "INVITING " .. bots[2]:name() .. " TO GROUP")
            if bots[0]:invite_group(bots[2]) == false then
                progress(leader, "FAILED TO INVITE " .. bots[2]:name() .. " TO GROUP")
                return false
            end
            progress(leader, "SUCCESSFULLY INVITED " .. bots[2]:name() .. " TO GROUP")

            progress(bots[1], bots[1]:name() .. " LEAVING GROUP VOLUNTARILY")
            if bots[1]:leave_group() == false then
                progress(bots[1], "FAILED TO LEAVE GROUP")
                return false
            end
            progress(bots[1], bots[1]:name() .. " SUCCESSFULLY LEFT GROUP")

            progress(leader, "GROUP MASTER KICKING " .. bots[2]:name())
            if bots[0]:kick_group(bots[2]) == false then
                progress(leader, "FAILED TO KICK " .. bots[2]:name() .. " FROM GROUP")
                return false
            end
            progress(leader, "SUCCESSFULLY KICKED " .. bots[2]:name() .. " FROM GROUP")

            progress(leader, "RE-INVITING " .. bots[1]:name() .. " TO GROUP")
            if bots[0]:invite_group(bots[1]) == false then
                progress(leader, "FAILED TO RE-INVITE " .. bots[1]:name() .. " TO GROUP")
                return false
            end
            progress(leader, "SUCCESSFULLY RE-INVITED " .. bots[1]:name() .. " TO GROUP")

            progress(leader, "TESTING NON-MASTER " .. bots[1]:name() .. " CANNOT INVITE " .. bots[2]:name())
            if bots[1]:invite_group(bots[2]) then
                progress(leader, "FAILED: NON-MASTER INVITE SUCCEEDED (UNEXPECTED)")
                return false
            end
            progress(leader, "CORRECTLY PREVENTED NON-MASTER FROM INVITING TO GROUP")

            progress(leader, "GROUP MASTER DISBANDING GROUP")
            local member_disbanded = false
            ctx:hook("message", function(_, bot, packet)
                if bot:name() == bots[1]:name()
                    and packet.type == "STATE"
                    and packet.text == "그룹 해체" then
                    member_disbanded = true
                end
            end)

            if lib.group.disband(bots[0]) == false then
                ctx:unhook("message")
                progress(leader, "FAILED TO DISBAND GROUP")
                return false
            end
            progress(leader, "GROUP MASTER SUCCESSFULLY DISBANDED GROUP")

            local waited_ms = 0
            local max_wait_ms = 3000
            while member_disbanded == false and waited_ms < max_wait_ms do
                ctx:sleep(skill.DEFAULT_INTERVAL)
                waited_ms = waited_ms + skill.DEFAULT_INTERVAL
            end
            ctx:unhook("message")

            if member_disbanded == false then
                progress(leader, "FAILED: MEMBER DID NOT RECEIVE DISBAND MESSAGE")
                return false
            end
            progress(bots[1], bots[1]:name() .. " RECEIVED DISBAND MESSAGE")

            progress(leader, "VERIFYING NEW GROUP CAN BE FORMED AFTER DISBAND")
            if bots[0]:invite_group(bots[2]) == false then
                progress(leader, "FAILED TO INVITE AFTER DISBAND")
                return false
            end
            progress(leader, "SUCCESSFULLY FORMED NEW GROUP AFTER DISBAND")

            progress(leader, "GROUP SCENARIO 2 TEST COMPLETED SUCCESSFULLY")
            return true
        end,
    },
}
