local go     = require("brain.trees.go")
local combat = require("brain.data.combat")
local facts  = require("sim.facts")

local TRIPS = {
    { from = { map = 22, x = 15, y = 107 }, to = 906, name = "국내성-서쪽해안가" },
    { from = { map = 22, x = 15, y = 107 }, to = 91, name = "국내성-전갈굴1" },
    { from = { map = 22, x = 15, y = 107 }, to = 412, name = "국내성-사마귀굴1" },
    { from = { map = 22, x = 15, y = 107 }, to = 6370, name = "국내성-개주사마귀굴1" },
    { from = { map = 14, x = 8, y = 89 }, to = 906, name = "부여성-서쪽해안가" },
    { from = { map = 14, x = 8, y = 89 }, to = 420, name = "부여성-국내해골굴1" },
    { from = { map = 906, x = 24, y = 90 }, to = 412, name = "서쪽해안가-사마귀굴1" },
    { from = { map = 91, x = 15, y = 15 }, to = 22, name = "전갈굴1-국내성" },
    { from = { map = 22, x = 15, y = 107 }, to = 673, name = "국내성-대방성" },
    { from = { map = 22, x = 15, y = 107 }, to = 860, name = "국내성-낙양성" },
    { from = { map = 22, x = 15, y = 107 }, to = 3050, name = "국내성-낙양살쾡이굴1" },
    { from = { map = 860, x = 20, y = 20 }, to = 22, name = "낙양성-국내성" },
    { from = { map = 3050, x = 15, y = 15 }, to = 412, name = "낙양살쾡이굴1-사마귀굴1" },
    { from = { map = 862, x = 20, y = 20 }, to = 91, name = "장안성-전갈굴1" },
}

local KITS = {
    { name = "맨몸", spells = {}, items = {} },
    { name = "비영사천문", spells = { "비영사천문" }, items = {} },
    { name = "귀환", spells = { "귀환" }, items = {} },
    { name = "노란비서", spells = {}, items = { "노란비서" } },
    { name = "전부", spells = { "비영사천문", "귀환" }, items = { "노란비서" } },
}

local STEP_LIMIT = 30

local function snapshot_of(kit, place)
    local spells = {}
    for slot, name in ipairs(kit.spells) do
        table.insert(spells, { slot = slot, name = name })
    end
    local items = {}
    for slot, name in ipairs(kit.items) do
        table.insert(items, { slot = slot, name = name, count = 10 })
    end
    return {
        self   = { name = "route", level = 63, class = "WARRIOR", nation = 1, ghost = false, hp = 4000, max_hp = 4000, mp = 1000, max_mp = 1000 },
        world  = { map = place.map, x = place.x, y = place.y, mobs = {} },
        spells = spells,
        bag    = { items = items, used = #items, capacity = 52 },
        pace   = { step = combat.walk },
        record = { memory = {}, failures = {} },
        gear   = {},
        now    = 0,
    }
end

local function journey(mind, kit, trip)
    local place = { map = trip.from.map, x = trip.from.x, y = trip.from.y }
    local legs  = {}
    local time  = 0
    local spent = {}
    for _ = 1, STEP_LIMIT do
        if place.map == trip.to then
            return legs, time, true
        end
        local snap   = snapshot_of(kit, place)
        local bound  = facts.bind(mind, snap)
        local detour = { closed = {}, perilous = {}, shunned = {}, spent = spent }
        local chosen = go.choose(snap, bound, { maps = { trip.to } }, detour)
        if chosen ~= nil then
            table.insert(legs, string.format("[%s]%d", chosen.name, chosen.dest))
            spent[chosen.name] = nil
            time = time + combat.transfer / 1000
            place = { map = chosen.dest, x = chosen.x, y = chosen.y }
        else
            local route = mind:route(nil, { maps = { trip.to } }, { perilous = {}, shunned = {} },
                { map = place.map, x = place.x, y = place.y, level = 63 })
            if route == nil then
                return legs, time, false
            end
            local exit = (route.path or {})[1]
            if exit ~= nil and exit.dest ~= nil and exit.dest ~= trip.to then
                local steps = math.abs((exit.x or place.x) - place.x) + math.abs((exit.y or place.y) - place.y)
                time = time + steps * combat.walk / 1000
                table.insert(legs, string.format("%d(%d칸%s)", exit.dest, steps, exit.world == true and ",월드맵" or ""))
                place = { map = exit.dest, x = exit.ax or route.x, y = exit.ay or route.y }
            else
                time = time + route.cost * combat.walk / 1000
                table.insert(legs, string.format("%d(%d칸)", trip.to, route.cost))
                place = { map = trip.to, x = route.x, y = route.y }
            end
        end
    end
    return legs, time, false
end

test_suite {
    name      = "Brain Route Test",
    bot_count = 0,

    scenarios = {
        function()
            local mind = brain.new()
            for _, trip in ipairs(TRIPS) do
                for _, kit in ipairs(KITS) do
                    local legs, time, arrived = journey(mind, kit, trip)
                    log("debug",
                        string.format("[brain:route] %s %s %s %.0fs %s", trip.name, kit.name, arrived and "도착" or "실패",
                            time, table.concat(legs, " > ")))
                end
            end
            return true
        end,
    },
}
