local protocol     = require("integration.protocol")
local script_step  = require("integration.lib.script_step")
local quest_dialog = require("integration.lib.quest_dialog")

local NPC = "마동참가신청"

local function fail(message)
    log("fatal", "Madong apply test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function open_form(bot)
    local oid = quest_dialog.use_npc(bot, NPC)
    if bot:request_dialog_ext(protocol.click(oid), quest_dialog.contains("1주일간")) == nil then
        return nil
    end
    if quest_dialog.next_dialog(bot, quest_dialog.contains("64팀")) == nil then
        return nil
    end
    if quest_dialog.next_dialog(bot, quest_dialog.contains("2인 1조")) == nil then
        return nil
    end
    if quest_dialog.next_dialog(bot, quest_dialog.contains("파트너 동의")) == nil then
        return nil
    end
    return quest_dialog.next_dialog(bot, quest_dialog.is_list)
end

local function find_option(list_packet, prefix)
    for _, text in ipairs(list_packet.list_lists or {}) do
        if text:find(prefix, 1, true) == 1 then
            return text
        end
    end
    return nil
end

local function select_form(bot, form, prefix, expect)
    local option = find_option(form, prefix)
    if option == nil then
        return nil
    end
    return quest_dialog.select_option(bot, form, option, expect)
end

test_suite {
    name      = "Madong Apply Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        quest_dialog.close(bot)
        quest_dialog.remove_npc(bot)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if run_step(bot, "madong_prepare", "madong_prepare") == false then
                return fail("could not prepare the character")
            end

            local form = open_form(bot)
            if form == nil then
                return fail("the apply form did not open")
            end
            local classes = select_form(bot, form, "클래스", quest_dialog.is_list)
            if classes == nil then
                return fail("the class list did not open")
            end
            if quest_dialog.select_option(bot, classes, "신선", quest_dialog.is_list) == nil then
                return fail("choosing the last class did not reopen the form")
            end
            if run_step(bot, "verify_madong_param", "madong_last_class", "신선||") == false then
                return fail("the last class was not kept")
            end
            quest_dialog.close(bot)

            form = open_form(bot)
            if form == nil then
                return fail("the apply form did not reopen")
            end
            local schedules = select_form(bot, form, "참여일정", quest_dialog.is_list)
            if schedules == nil then
                return fail("the schedule list did not open")
            end
            if quest_dialog.select_option(bot, schedules, "1차예선 19일 / 2차예선 26일", quest_dialog.is_list) == nil then
                return fail("choosing the last schedule did not reopen the form")
            end
            if run_step(bot, "verify_madong_param", "madong_last_schedule", "신선||19/26") == false then
                return fail("the last schedule was not kept")
            end
            return true
        end,
    },
}
