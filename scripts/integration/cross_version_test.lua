local lib = require("integration.lib")
local resp = require("integration.response")
local protocol = require("integration.protocol")
local trade = require("integration.lib.trade")

local CLIENTS = {
    { version = 550, ui_mode = "OLD" },
    { version = 651, ui_mode = "NEW" },
    { version = 651, ui_mode = "OLD" },
}

local CHECKED_OPCODES = {
    [0x0D] = "chat",
    [0x1D] = "update_external",
    [0x2F] = "dialog",
    [0x38] = "item_unequip",
    [0x42] = "trade",
}

local WAIT_STEPS = 50
local WAIT_INTERVAL = 100
local MESSAGE_TRADE_SUCCESS = "교환에 성공했습니다."

local function describe(bot)
    return string.format("slot=%s client=%d/%s", tostring(bot:slot()), bot:client_version(), bot:ui_mode())
end

local function fail(message)
    log("fatal", "cross_version_test FAILED: " .. message)
    return false
end

local function wait_until(ctx, predicate)
    for _ = 1, WAIT_STEPS do
        if predicate() then
            return true
        end
        ctx:sleep(WAIT_INTERVAL)
    end
    return predicate()
end

local function observe_weapon(ctx, wearer, item_name)
    local seen = {}
    ctx:hook("update_external", function(_, viewer, packet)
        if viewer:oid() ~= wearer:oid() and packet.oid == wearer:oid() and packet.weapon_dress ~= 0xFFFF then
            seen[viewer:slot()] = packet
        end
    end)

    wearer:clear_inventory()
    wearer:create_item(item_name, 1)
    local equipped = wearer:equip(0)

    local viewers = ctx:bot_count() - 1
    wait_until(ctx, function()
        local count = 0
        for _ in pairs(seen) do
            count = count + 1
        end
        return count == viewers
    end)
    ctx:unhook("update_external")

    if equipped == false then
        return fail(string.format("%s could not equip %s", describe(wearer), item_name))
    end

    local weapon = nil
    for i = 0, ctx:bot_count() - 1 do
        local viewer = ctx:bot(i)
        if viewer:oid() ~= wearer:oid() then
            local packet = seen[i]
            if packet == nil then
                return fail(string.format("%s did not see %s equip %s", describe(viewer), describe(wearer), item_name))
            end
            if viewer:client_version() == 550 and packet.name ~= wearer:name() then
                return fail(
                    string.format(
                        "%s read name '%s' for %s named '%s'",
                        describe(viewer),
                        tostring(packet.name),
                        describe(wearer),
                        wearer:name()
                    )
                )
            end
            if weapon ~= nil and packet.weapon_dress ~= weapon then
                return fail(
                    string.format(
                        "%s read weapon %d, another viewer read %d",
                        describe(viewer),
                        packet.weapon_dress,
                        weapon
                    )
                )
            end
            weapon = packet.weapon_dress
        end
    end

    wearer:unequip("WEAPON")
    wearer:clear_inventory()
    return true
end

local function open_shop(ctx, bot)
    bot:direction("BOTTOM")
    bot:move("BOTTOM")
    local npc = bot:create_npc("뭉치")
    bot:move("TOP")
    bot:direction("BOTTOM")

    local pursuit = bot:request_dialog(protocol.click(npc.oid), function(packet)
        return packet.type == "pursuit"
    end)
    if pursuit == nil or pursuit.oid ~= npc.oid or #pursuit.menu_menus == 0 then
        return fail(string.format("%s did not read the shop pursuit dialog", describe(bot)))
    end

    bot:request_dialog(protocol.dialog("PURSUIT", 0, "", 0, 0, "물건 사기"), function(packet)
        return packet.type == "pursuit"
    end)

    local items = bot:request_dialog(protocol.dialog("PURSUIT", 0, "", 0, 0, "전사용 갑주류"), function(packet)
        return packet.type == "item"
    end)
    bot:send(protocol.dialog("ITEM", 0, "", 0, 0, "unknown"))

    if items == nil or items.oid ~= npc.oid or #items.item_items == 0 then
        return fail(string.format("%s did not read the shop item dialog", describe(bot)))
    end
    for _, item in ipairs(items.item_items) do
        if item.name == "" then
            return fail(string.format("%s read an item without a name", describe(bot)))
        end
    end
    return true
end

test_suite({
    name = "Cross Version Test",
    clients = { 550, "651:new", "651" },

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            bot:clear_inventory()
            bot:money(0)
        end
    end,

    scenarios = {
        function(ctx)
            if ctx:bot_count() ~= #CLIENTS then
                return fail(string.format("expected %d bots, got %d", #CLIENTS, ctx:bot_count()))
            end

            for i = 0, ctx:bot_count() - 1 do
                local bot = ctx:bot(i)
                local expected = CLIENTS[i + 1]
                if bot:slot() ~= i then
                    return fail(string.format("ctx:bot(%d) is %s", i, describe(bot)))
                end
                if bot:client_version() ~= expected.version or bot:ui_mode() ~= expected.ui_mode then
                    return fail(
                        string.format(
                            "ctx:bot(%d) is %s, expected %d/%s",
                            i,
                            describe(bot),
                            expected.version,
                            expected.ui_mode
                        )
                    )
                end
                if bot:oid() == 0 or bot:map() == 0xFFFF then
                    return fail(string.format("%s did not finish login", describe(bot)))
                end
            end
            return true
        end,

        function(ctx)
            for s = 0, ctx:bot_count() - 1 do
                local sender = ctx:bot(s)
                for l = 0, ctx:bot_count() - 1 do
                    if l ~= s then
                        local listener = ctx:bot(l)
                        local text = string.format("cross-version %d to %d", s, l)
                        local packet = sender:request_on(listener, resp.chat, protocol.chat(false, text), function(p)
                            return p.oid == sender:oid() and string.find(p.text or "", text, 1, true) ~= nil
                        end)
                        if packet == false or packet == nil then
                            return fail(string.format("%s did not hear %s", describe(listener), describe(sender)))
                        end
                    end
                end
            end
            return true
        end,

        function(ctx)
            for i = 0, ctx:bot_count() - 1 do
                if observe_weapon(ctx, ctx:bot(i), "목도") == false then
                    return false
                end
            end
            return true
        end,

        function(ctx)
            for i = 0, ctx:bot_count() - 1 do
                if open_shop(ctx, ctx:bot(i)) == false then
                    return false
                end
            end
            return true
        end,

        function(ctx)
            local legacy = ctx:bot(0)
            local modern = ctx:bot(1)

            modern:create_item("도토리", 50)
            legacy:money(1000)

            if trade.request(modern, legacy:oid(), trade.dialog_oid(legacy:oid())) == nil then
                return fail(string.format("%s did not open a trade with %s", describe(modern), describe(legacy)))
            end
            trade.up_item(modern, legacy:oid(), 0)
            trade.item_count(modern, legacy:oid(), 50)
            trade.up_money(legacy, modern:oid(), 1000)
            trade.lock(modern, legacy:oid(), trade.type_is("lock"))
            if trade.lock(legacy, modern:oid(), trade.close_contains(MESSAGE_TRADE_SUCCESS)) == nil then
                return fail("trade between 5.50 and 6.51 did not succeed")
            end

            lib.wait.state(legacy, resp.item_update, function()
                return legacy:item_count("도토리") == 50
            end)
            lib.wait.state(modern, resp.update_internal, function()
                return modern:money() == 1000
            end)

            if legacy:item_count("도토리") ~= 50 or modern:money() ~= 1000 then
                return fail(
                    string.format(
                        "trade result: %s has %d 도토리, %s has %d money",
                        describe(legacy),
                        legacy:item_count("도토리"),
                        describe(modern),
                        modern:money()
                    )
                )
            end
            return true
        end,

        function(ctx)
            local ok = true
            local legacy = ctx:bot(0):unread_opcodes()
            for i = 0, ctx:bot_count() - 1 do
                local bot = ctx:bot(i)
                for opcode, count in pairs(bot:unread_opcodes()) do
                    log(
                        "debug",
                        string.format(
                            "cross_version_test: %s misread %d packet(s) of opcode 0x%02X",
                            describe(bot),
                            count,
                            opcode
                        )
                    )
                    if CHECKED_OPCODES[opcode] ~= nil then
                        ok = fail(
                            string.format(
                                "%s misread %s (opcode 0x%02X)",
                                describe(bot),
                                CHECKED_OPCODES[opcode],
                                opcode
                            )
                        )
                    elseif legacy[opcode] == nil then
                        ok =
                            fail(string.format("%s misread opcode 0x%02X that 5.50 reads fully", describe(bot), opcode))
                    end
                end
            end
            return ok
        end,
    },
})
