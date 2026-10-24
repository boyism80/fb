local resp         = require("integration.response")
local protocol     = require("integration.protocol")
local script_step  = require("integration.lib.script_step")
local quest_dialog = require("integration.lib.quest_dialog")

local MSG_MULTIPLIER_RANGE = "배율은 0~100 사이의 숫자여야 합니다."

local REVIVE_NPC         = "좌성황"
local MENU_YES           = 1
local MSG_REVIVE_GRANTED = "너의 각오를 믿고"
local REVIVED_HP         = 33

local function fail(message)
    log("fatal", "Character rule test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function is_menu(p)
    return p.type == "menu"
end

local function answer_yes()
    return protocol.dialog("MENU", 0, "", MENU_YES, 0, "", "NEXT")
end

local function revive_after_oath(ctx, bot, oid)
    if bot:request_dialog(protocol.click(oid), is_menu) == nil then
        return "the revive NPC did not ask the first oath"
    end
    for i = 2, 4 do
        if bot:request_dialog(answer_yes(), is_menu) == nil then
            return string.format("the revive NPC did not ask oath %d", i)
        end
    end
    local granted = bot:request_dialog_ext(answer_yes(), quest_dialog.contains(MSG_REVIVE_GRANTED))
    if granted == nil then
        return "the revive NPC did not grant the revive"
    end

    if run_step(bot, "revive_with_hp", "revive_elsewhere", tostring(REVIVED_HP)) == false then
        return "could not revive the bot during the dialog"
    end
    quest_dialog.press(bot, "NEXT")
    ctx:sleep(500)
    if run_step(bot, "verify_alive_hp", "revive_kept_hp", tostring(REVIVED_HP)) == false then
        return "the revive NPC overwrote the HP of a character revived during the dialog"
    end
    return nil
end

test_suite {
    name      = "Character Rule Test",
    bot_count = 2,

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

        function(ctx)
            local bot = ctx:bot(0)
            if run_step(bot, "verify_caster_state", "caster_state") == false then
                return fail("a ghost or riding caster cast an area spell")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            if run_step(bot, "verify_uncloak", "uncloak") == false then
                return fail("a cloak spell overwrote a state it did not set")
            end
            return true
        end,

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            if run_step(a, "verify_heal_ghost", "heal_ghost", b:name()) == false then
                return fail("a heal spell healed a ghost")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            local oid = bot:create_npc(REVIVE_NPC).oid
            local err = nil
            if run_step(bot, "set_ghost", "revive_ghost_on", "true") == false then
                err = "could not make the bot a ghost"
            else
                err = revive_after_oath(ctx, bot, oid)
            end
            run_step(bot, "remove_npc", "revive_npc_removed", REVIVE_NPC)
            run_step(bot, "set_ghost", "revive_ghost_off", "false")
            if err ~= nil then
                return fail(err)
            end
            return true
        end,
    },
}
