local protocol     = require("integration.protocol")
local resp         = require("integration.response")
local quest_dialog = require("integration.lib.quest_dialog")

local NPC    = "떡쇠"
local WEAPON = "목도"
local NAME   = "나의목도"
local MONEY  = 1000

local MSG_CHANGED = "장비가 바뀌었습니다"
local MSG_RENAMED = "변경했습니다"

local function fail(message)
    log("fatal", "Weapon rename test: " .. message)
    return false
end

local function is_type(name)
    return function(p)
        return p.type == name
    end
end

-- Returns the inventory slot (1-based) once the cost confirm is shown, or nil.
local function open_confirm(bot)
    local oid = quest_dialog.use_npc(bot, NPC)
    if bot:request_dialog(protocol.click(oid), is_type("pursuit")) == nil then
        return nil, "the shop menu did not open"
    end
    if bot:request_dialog(protocol.dialog("PURSUIT", 0, "", 0, 0, "무기 이름 명명"), is_type("slot")) == nil then
        return nil, "the weapon slot dialog did not open"
    end

    local slot = bot:item_slot(WEAPON) + 1
    if bot:request_dialog(protocol.dialog("SLOT", 0, "", slot, 0, "", "NEXT"), is_type("input")) == nil then
        return nil, "the name input did not open"
    end
    local confirm = bot:request_dialog(protocol.dialog("INPUT", 0, NAME, 0, 0, "", "NEXT"), function(p)
        return p.message ~= nil and p.message:find("정말 바꾸시겠습니까", 1, true) ~= nil
    end)
    if confirm == nil then
        return nil, "the cost confirm did not open"
    end
    return slot
end

test_suite {
    name      = "Weapon Rename Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        quest_dialog.close(bot)
        quest_dialog.remove_npc(bot)
        bot:clear_inventory()
    end,

    scenarios = {
        -- Dropping the weapon before confirming must neither charge nor rename.
        function(ctx)
            local bot = ctx:bot(0)
            bot:clear_inventory()
            bot:money(MONEY)
            bot:create_item(WEAPON, 1)
            if bot:has_item_by_name(WEAPON) == false then
                return fail("the weapon was not created")
            end

            local slot, err = open_confirm(bot)
            if slot == nil then
                return fail(err)
            end

            local dropped = bot:request(resp.item_remove, protocol.item_drop(slot, false), function(p)
                return p.index == slot - 1
            end)
            if dropped == nil or dropped == false then
                return fail("the weapon was not dropped")
            end

            local answer = bot:request_dialog_ext(protocol.dialog("MENU", 0, "", 1, 0, "", "NEXT"), function(p)
                return p.message ~= nil
                    and (p.message:find(MSG_CHANGED, 1, true) ~= nil or p.message:find(MSG_RENAMED, 1, true) ~= nil)
            end)
            if answer == nil then
                return fail("no rename answer")
            end
            if answer.message:find(MSG_RENAMED, 1, true) ~= nil then
                return fail("a dropped weapon was renamed: " .. answer.message)
            end

            ctx:sleep(300)
            if bot:money() ~= MONEY then
                return fail(string.format("money was charged money=%d expected=%d", bot:money(), MONEY))
            end
            return true
        end,

        -- A weapon still held is renamed and charged once.
        function(ctx)
            local bot = ctx:bot(0)
            bot:clear_inventory()
            bot:money(MONEY)
            bot:create_item(WEAPON, 1)

            local slot, err = open_confirm(bot)
            if slot == nil then
                return fail(err)
            end

            local answer = bot:request_dialog_ext(protocol.dialog("MENU", 0, "", 1, 0, "", "NEXT"),
                quest_dialog.contains(MSG_RENAMED))
            if answer == nil then
                return fail("the held weapon was not renamed")
            end

            ctx:sleep(300)
            if bot:money() ~= MONEY - 10 then
                return fail(string.format("rename cost money=%d expected=%d", bot:money(), MONEY - 10))
            end
            return true
        end,
    },
}
