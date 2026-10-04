local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

-- A swap sends the src slot packet first and the dst slot packet last on the same socket,
-- so each request waits for the dst packet and the src packet is already applied.

local function test_item_swap_1(ctx, index)
    log("debug", "Starting test_item_swap_1")
    local bot = ctx:bot(index)

    bot:create_item("도토리", 200)
    bot:create_item("토끼고기", 100)

    bot:request(
        resp.item_update,
        protocol.swap("ITEM", 1, 2),
        function(packet)
            return packet.index == 1
        end)

    if bot:item_slot("도토리") ~= 1 then
        return false
    end
    if bot:item_slot("토끼고기") ~= 0 then
        return false
    end

    return true
end

local function test_item_swap_2(ctx, index)
    log("debug", "Starting test_item_swap_2")
    local bot = ctx:bot(index)

    bot:request(
        resp.item_update,
        protocol.swap("ITEM", 1, 3),
        function(packet)
            return packet.index == 2
        end)

    if bot:has_item(0) then
        return false
    end

    return true
end

local function test_item_swap_3(ctx, index)
    log("debug", "Starting test_item_swap_3")
    local bot = ctx:bot(index)

    bot:request(
        resp.item_remove,
        protocol.swap("ITEM", 1, 3),
        function(packet)
            return packet.index == 2
        end)

    if bot:has_item(2) then
        return false
    end

    return true
end

local function test_spell_swap_1(ctx, index)
    log("debug", "Starting test_spell_swap_1")
    local bot = ctx:bot(index)

    bot:learn_spell("누리의기원")
    bot:learn_spell("바다의기원")

    bot:request(
        resp.spell_update,
        protocol.swap("SPELL", 1, 2),
        function(packet)
            return packet.index == 1
        end)

    if bot:spell_slot("누리의기원") ~= 1 then
        return false
    end
    if bot:spell_slot("바다의기원") ~= 0 then
        return false
    end

    return true
end

local function test_spell_swap_2(ctx, index)
    log("debug", "Starting test_spell_swap_2")
    local bot = ctx:bot(index)

    bot:request(
        resp.spell_update,
        protocol.swap("SPELL", 1, 3),
        function(packet)
            return packet.index == 2
        end)

    if bot:spell_slot("바다의기원") ~= 2 then
        return false
    end

    return true
end

local function test_spell_swap_3(ctx, index)
    log("debug", "Starting test_spell_swap_3")
    local bot = ctx:bot(index)

    bot:request(
        resp.spell_remove,
        protocol.swap("SPELL", 1, 3),
        function(packet)
            return packet.index == 2
        end)

    if bot:has_spell(2) then
        return false
    end
    if bot:spell_slot("바다의기원") ~= 0 then
        return false
    end

    return true
end

test_suite {
    name      = "Swap Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Swap test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    scenarios = {
        {
            parallel = {
                [0] = {
                    function(ctx) return test_item_swap_1(ctx, 0) end,
                    function(ctx) return test_item_swap_2(ctx, 0) end,
                    function(ctx) return test_item_swap_3(ctx, 0) end,
                },
                [1] = {
                    function(ctx) return test_spell_swap_1(ctx, 1) end,
                    function(ctx) return test_spell_swap_2(ctx, 1) end,
                    function(ctx) return test_spell_swap_3(ctx, 1) end,
                },
            },
        },
    },
}
