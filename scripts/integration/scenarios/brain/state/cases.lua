local expect   = require("integration.lib.expect")
local gear     = require("integration.lib.gear")
local protocol = require("integration.protocol")
local quest    = require("integration.lib.quest_dialog")
local snapshot = require("brain.snapshot")
local decide   = require("brain.decide")
local combat   = require("brain.policies.combat")
local score    = require("brain.policies.score")
local ground   = require("brain.policies.ground")
local query    = require("brain.query")
local profiles = require("integration.scenarios.brain.state.profiles")
local resp     = require("integration.response")

local M = {}

local TIMEOUT           = 120000
local START             = { map = "국내성", x = 15, y = 107 }
local STRIKER           = { hp = 10 }
local SUPPLIER          = { money = 1000, mp = 300 }
local SELLER            = { spare = "목도" }
local UNREACHABLE_WATCH = 3000

local spot = START

function M.start(x, y)
    spot = { map = START.map, x = x, y = y }
end

local function enter(ctx, index)
    local bot     = ctx:bot(index)
    local current = id2map(bot:map())
    bot:map_move(spot.map, spot.x, spot.y)
    if current == nil or current:name() ~= spot.map then
        if expect.arrive(ctx, index, spot.map, TIMEOUT) == false then
            return nil
        end
    end
    return ctx:bot(index)
end

local function stage(ctx, index)
    local bot = ctx:bot(index)
    gear.strip(bot, { "WEAPON" })
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("전사")
    bot:chat("/레벨바꾸기 5")
    bot:chat("/힘바꾸기 10")
    return enter(ctx, index)
end

local function history(mind, bot)
    local lines = {}
    local tasks = {}
    for _, entry in ipairs(mind:states(bot) or {}) do
        table.insert(lines, string.format("%s/%s(%s)", entry.name, entry.task, entry.reason))
        tasks[entry.task] = (tasks[entry.task] or 0) + 1
    end
    return table.concat(lines, " -> "), tasks
end

local function finish(ctx, index, mind, name, passed, detail)
    local summary = string.format("%s history=%s", detail, (history(mind, ctx:bot(index))))
    expect.release(ctx, index, mind, TIMEOUT)
    if passed then
        log("debug", string.format("[brain:state:%s] SUCCESS %s", name, summary))
    else
        log("fatal", string.format("[brain:state:%s] FAILED %s", name, summary))
    end
    return passed
end

function M.idle(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1 })
    local idle = expect.within(ctx, TIMEOUT, function()
        local state = mind:state(ctx:bot(index))
        return state ~= nil and state.reason == "idle"
    end)
    return finish(ctx, index, mind, "idle", idle, "idle=" .. tostring(idle))
end

function M.equip(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    bot:create_item("목도", 1)
    bot:equip(bot:item_slot("목도"))
    bot:create_item("목검", 1)

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1 })
    local worn = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return current:item_count("목검") == 0 and current:item_count("목도") == 1
    end)
    local _, tasks = history(mind, ctx:bot(index))
    return finish(ctx, index, mind, "equip", worn and tasks.EQUIP ~= nil, "worn=" .. tostring(worn))
end

function M.unreachable(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    ctx:bot(index):chat("/레벨바꾸기 1")
    local mind  = brain.new()
    local order = { task = "GO", dest = { map = "국내문파대결장", x = 15, y = 3 } }
    ctx:bot(index):brain(mind, { seed = 1, memory = { order = order } })
    local recorded = expect.within(ctx, TIMEOUT, function()
        local record = mind:failure(ctx:bot(index), "ORDER")
        return record ~= nil and record.reason == "NO_ROUTE"
    end)
    local first    = mind:failure(ctx:bot(index), "ORDER")
    ctx:sleep(UNREACHABLE_WATCH)
    local record  = mind:failure(ctx:bot(index), "ORDER")
    local retries = first ~= nil and record ~= nil and record.failures - first.failures or 0
    local allowed = first ~= nil and 1 + math.log(UNREACHABLE_WATCH / math.max(first.duration, 1), 2) or 0
    local passed  = recorded and record ~= nil and retries <= allowed
    return finish(ctx, index, mind, "unreachable", passed,
        string.format("recorded=%s retries=%d allowed=%.1f", tostring(recorded), retries, allowed))
end

function M.reattach(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, memory = { note = 7 } })
    expect.release(ctx, index, mind, TIMEOUT)
    ctx:bot(index):brain(mind, { seed = 1 })
    expect.within(ctx, TIMEOUT, function()
        return mind:state(ctx:bot(index)) ~= nil
    end)
    local note = mind:recall(ctx:bot(index), "note")
    return finish(ctx, index, mind, "reattach", note == 7, "note=" .. tostring(note))
end

function M.snapshot(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    bot:create_item("도토리", 1)
    bot:learn_spell("유인")
    bot:set_current_hp_mp(30, 20)

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, memory = { note = 7 } })
    expect.within(ctx, TIMEOUT, function()
        return mind:state(ctx:bot(index)) ~= nil
    end)

    local current = ctx:bot(index)
    local snap    = snapshot.take(mind, current)
    local bagged  = false
    for _, item in ipairs(snap.bag.items) do
        bagged = bagged or item.name == "도토리"
    end
    local learned = false
    for _, spell in ipairs(snap.spells) do
        learned = learned or spell.name == "유인"
    end
    local checks = {
        self   = snap.self.level == current:level() and snap.self.class == "WARRIOR",
        vitals = snap.self.hp == current:hp() and snap.self.mp == current:mp(),
        money  = snap.self.money == current:money(),
        bag    = bagged,
        spells = learned,
        world  = snap.world.map == current:map() and type(snap.world.mobs) == "table",
        record = snap.record.current ~= nil and snap.record.memory.note == 7,
        gear   = type(snap.gear) == "table",
    }
    local passed = true
    local failed = {}
    for name, ok in pairs(checks) do
        passed = passed and ok
        if ok == false then
            table.insert(failed, name)
        end
    end
    return finish(ctx, index, mind, "snapshot", passed, "failed=" .. table.concat(failed, ","))
end

function M.dialog(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    local oid  = quest.use_npc(bot, "좌성황")
    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1 })
    ctx:bot(index):send(protocol.click(oid))
    local closed = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        local _, tasks = history(mind, current)
        return tasks.DIALOG ~= nil and mind:observe(current).dialog == false
    end)
    local passed = finish(ctx, index, mind, "dialog", closed, "closed=" .. tostring(closed))
    quest.remove_npc(ctx:bot(index))
    return passed
end

local function fake(changes)
    local snap = {
        self   = { level = 5, class = "WARRIOR", nation = 1, ghost = false, hp = 100, max_hp = 100, money = 0 },
        bag    = { items = {}, used = 0, capacity = 26 },
        gear   = {},
        world  = { map = 1, x = 10, y = 10, mobs = {}, items = {}, dialog = false },
        record = { failures = {}, memory = {} },
        pace   = {},
    }
    changes(snap)
    return snap
end

local function hurt(changes)
    return fake(function(s)
        s.self.hp             = 1
        s.record.memory.order = { task = "HUNT", names = { "다람쥐" }, count = 1 }
        changes(s)
    end)
end

local function drained(changes)
    return fake(function(s)
        s.self.class  = "MAGE"
        s.self.mp     = 0
        s.self.max_mp = 300
        s.spells      = { { slot = 1, name = "화염주" } }
        changes(s)
    end)
end

function M.pure(ctx, index)
    local facts  = {
        npcs = function()
            return { { name = "좌성황", map = 1, x = 11, y = 10 } }
        end,
        item = function()
            return nil
        end,
        spawns = function()
            return {}
        end,
        habitats = function()
            return { { id = 2, name = "부여왕초보사냥1" } }
        end,
        route = function()
            return { cost = 1, dest = 1 }
        end,
        path = function()
            return { cost = 1, dest = 1 }
        end,
        native = function()
            return false
        end,
        demand = function()
            return 0
        end,
    }
    local cases  = {
        { expect = { "FIELD", ACTION.MOVE }, snap = fake(function() end) },
        { expect = { "REVIVE", ACTION.TALK }, snap = fake(function(s) s.self.ghost = true end) },
        {
            expect = { "REVIVE", ACTION.MOVE },
            snap = fake(function(s)
                s.self.ghost = true
                s.world.x = 30
            end)
        },
        { expect = { "DIALOG", ACTION.TALK }, snap = fake(function(s) s.world.dialog = true end) },
        { expect = { "FIELD", ACTION.MOVE }, snap = fake(function(s) s.bag.used = 26 end) },
        {
            expect = { "DEFEND", ACTION.ATTACK },
            snap = fake(function(s)
                s.self.struck = 100
                s.world.mobs  = { {
                    oid = 9,
                    name = "다람쥐",
                    x = 11,
                    y = 10,
                    hp = 15,
                    damage = 2,
                    least = 1,
                    speed = 2000,
                    attack = "RETALIATE",
                    size = "SMALL",
                    defense = 0,
                    resists = {}
                } }
            end)
        },
        { expect = { "ORDER", ACTION.MOVE }, snap = fake(function(s) s.record.memory.order = { task = "GO", dest = { map = 2 } } end) },
        { expect = { "REST", ACTION.SAY }, snap = hurt(function() end) },
        { expect = { "REST", ACTION.MOVE }, snap = hurt(function(s) s.world.x = 30 end) },
        {
            expect = { "REST", ACTION.USE },
            snap = hurt(function(s)
                s.world.x   = 30
                s.bag.items = { { name = "감자", count = 2, slot = 1 } }
            end)
        },
        {
            expect = { "REST", ACTION.CAST },
            snap = hurt(function(s)
                s.world.x     = 30
                s.self.mp     = 300
                s.self.max_mp = 300
                s.spells      = { { slot = 1, name = "누리의기원" } }
            end)
        },
        {
            expect = { "ORDER", ACTION.MOVE },
            snap = hurt(function(s)
                s.self.hp     = 150
                s.self.max_hp = 300
            end)
        },
        { expect = { "REST", ACTION.USE }, snap = drained(function(s) s.bag.items = { { name = "동동주", count = 2, slot = 1 } } end) },
        { expect = { "FIELD", ACTION.MOVE }, snap = drained(function() end) },
        { expect = { "SUPPLY", ACTION.SAY }, snap = drained(function(s) s.self.money = 1000 end) },
        { expect = { "FIELD", ACTION.MOVE }, snap = fake(function(s) s.self.money = 1000 end) },
        {
            expect = { "BUFF", ACTION.CAST },
            snap = fake(function(s)
                s.self.mp             = 100
                s.self.max_mp         = 100
                s.spells              = { { slot = 1, name = "신의축복" } }
                s.record.memory.order = { task = "HUNT", names = { "다람쥐" }, count = 1 }
            end)
        },
    }
    local failed = {}
    local mob    = {
        name = "다람쥐",
        hp = 60,
        damage = 10,
        least = 6,
        speed = 1000,
        attack = "RETALIATE",
        size = "SMALL",
        defense = 0,
        resist = 0
    }
    local armed  = {
        npcs = facts.npcs,
        item = function()
            return { type = "WEAPON", small = { min = 20, max = 30 }, large = { min = 30, max = 40 } }
        end,
    }
    local bare   = fake(function() end)
    local held   = fake(function(s) s.gear = { { type = "WEAPON", names = { "목검" } } } end)
    local model  = {
        combat.mitigate(100, 0, 0) == 100,
        combat.accuracy(12) == 1.0 and combat.accuracy(-5) == 0.1,
        combat.strike(held, armed, mob) > combat.strike(bare, facts, mob),
        combat.outcome(held, armed, mob).taken < combat.outcome(bare, facts, mob).taken,
        combat.outcome(bare, facts, mob).margin < combat.outcome(held, armed, mob).margin,
        combat.outcome(fake(function(s)
            s.self.mp = 1000
            s.spells = { { slot = 1, name = "극진뢰격참주" } }
        end), facts, mob).time <
        combat.outcome(bare, facts, mob).time,
        combat.outcome(fake(function(s)
                s.self.hp = 5
                s.self.mp = 300
                s.spells = { { slot = 1, name = "누리의기원" } }
            end), facts,
            mob).healed > 0,
    }
    local function equipped(changes)
        return fake(function(s)
            s.self.mp     = 1000
            s.self.max_mp = 1000
            changes(s)
        end)
    end
    local plain   = score.capability(equipped(function() end), facts)
    local mage    = score.capability(equipped(function(s)
        s.spells = { { slot = 1, name = "극진뢰격참주" }, { slot = 2, name = "공력증강" } }
    end), facts)
    local striker = score.capability(equipped(function(s) s.spells = { { slot = 1, name = "건곤대나이" } } end), facts)
    local healer  = score.capability(equipped(function(s)
        s.spells = { { slot = 1, name = "구름의기원" }, { slot = 2, name = "공력증강" } }
    end), facts)
    local stocked = score.capability(equipped(function(s) s.bag.items = { { name = "감자", count = 3, slot = 1 } } end),
        facts)
    local runner  = score.capability(equipped(function(s) s.spells = { { slot = 1, name = "비영사천문" } } end), facts)
    table.insert(model, mage.dps.single > plain.dps.single and mage.burst > plain.burst)
    table.insert(model, striker.burst > plain.burst and striker.dps.melee == plain.dps.melee)
    table.insert(model, healer.hps.others > 0 and plain.hps.others == 0 and plain.hps.self == 0)
    table.insert(model, stocked.supply.hp == 150 and plain.supply.hp == 0)
    table.insert(model, #runner.mobility == 1 and #plain.mobility == 0)
    local rogue   = equipped(function(s) s.spells = { { slot = 1, name = "삼중공격" } } end)
    local ambush  = equipped(function(s) s.spells = { { slot = 1, name = "투명" }, { slot = 2, name = "비영승보" } } end)
    local armored = equipped(function(s) s.spells = { { slot = 1, name = "무장" }, { slot = 2, name = "보호" } } end)
    local charger = equipped(function(s) s.spells = { { slot = 1, name = "공력증강" } } end)
    local base    = equipped(function() end)
    table.insert(model, combat.strike(rogue, facts, mob) > combat.strike(base, facts, mob) * 2.5)
    local function learned(names, weapon)
        return equipped(function(s)
            s.spells = {}
            for slot, name in ipairs(names) do
                table.insert(s.spells, { slot = slot, name = name })
            end
            if weapon then
                s.gear = { { type = "WEAPON", names = { "목검" } } }
            end
        end)
    end
    local tough = {
        name = "곰",
        hp = 3000,
        damage = 40,
        least = 30,
        speed = 1000,
        attack = "AGGRESSIVE",
        size = "LARGE",
        defense = 0,
        resists = {}
    }
    table.insert(model, combat.strike(learned({ "신의축복" }), facts, mob) > combat.strike(base, facts, mob))
    table.insert(model, combat.strike(learned({ "백호령" }), facts, mob) > combat.strike(base, facts, mob))
    table.insert(model,
        combat.strike(learned({ "대력검신" }, true), armed, mob) > combat.strike(learned({}, true), armed, mob))
    table.insert(model, combat.strike(learned({ "분신" }), facts, mob) > combat.strike(base, facts, mob) * 1.9)
    table.insert(model, combat.strike(learned({ "측면공격" }), facts, mob, 0.5) > combat.strike(base, facts, mob, 0.5))
    table.insert(model, combat.outcome(learned({ "중독" }), facts, tough).time < combat.outcome(base, facts, tough).time)
    table.insert(model, combat.outcome(learned({ "마비" }), facts, tough).taken < combat.outcome(base, facts, tough).taken)
    table.insert(model,
        combat.buffs(learned({ "운기" })).surge ~= nil and combat.buffs(learned({ "금강불체" })).emergency ~= nil)
    table.insert(model, combat.outcome(ambush, facts, mob).swings < combat.outcome(base, facts, mob).swings)
    table.insert(model, combat.blow(armored, mob) < combat.blow(base, mob))
    table.insert(model, combat.buffs(charger).charge ~= nil and combat.buffs(charger).charge.chance == 0.5 and
        combat.buffs(base).charge == nil)
    local native = false
    local danger = {
        npcs = facts.npcs,
        item = facts.item,
        native = function()
            return native
        end,
    }
    local function remembered(now)
        return fake(function(s)
            s.now = now
            s.record.memory.perils = { ["5"] = { mob = 432, at = 0 } }
        end)
    end
    local recent   = require("brain.policies.danger").shunned(remembered(1000), danger)
    local stale    = require("brain.policies.danger").shunned(remembered(10 ^ 9), danger)
    native         = true
    local resident = require("brain.policies.danger").shunned(remembered(10 ^ 9), danger)
    table.insert(model, #recent == 1 and recent[1] == 5 and #stale == 0 and #resident == 1)
    native         = false
    local stranger = require("brain.policies.danger").shunned(fake(function(s)
        s.now = 1000
        s.record.memory.perils = {
            ["7"] = {
                mob = 999999,
                at = 0,
                stats = {
                    id = 999999,
                    name = "낯선몹",
                    hp = 100000,
                    damage = 1000,
                    least = 1000,
                    speed = 100,
                    attack = "AGGRESSIVE",
                    size = "SMALL",
                    defense = 0,
                    resists = {}
                }
            }
        }
    end), danger)
    table.insert(model, #stranger == 1 and stranger[1] == 7)
    for number, ok in ipairs(model) do
        if ok == false then
            table.insert(failed, "model" .. number)
        end
    end
    for number, case in ipairs(cases) do
        local decision = decide.choose(case.snap, facts, nil)
        local ok       = case.expect == nil and decision == nil or
            decision ~= nil and case.expect ~= nil and decision.task == case.expect[1] and
            decision.kind == case.expect[2]
        if ok == false then
            table.insert(failed, string.format("%d:%s/%s", number, decision and decision.task or "-",
                decision and tostring(decision.kind) or "-"))
        end
    end
    local passed = #failed == 0
    log(passed and "debug" or "fatal", string.format("[brain:state:pure] %s failed=%s", passed and "SUCCESS" or "FAILED",
        table.concat(failed, ",")))
    return passed
end

local function profile(class, level, hp, mp, weapon, spells)
    return fake(function(s)
        s.self.class  = class
        s.self.level  = level
        s.self.hp     = hp
        s.self.max_hp = hp
        s.self.mp     = mp
        s.self.max_mp = mp
        s.gear        = weapon ~= nil and { { type = "WEAPON", names = { weapon } } } or {}
        s.spells      = {}
        for slot, name in ipairs(spells) do
            table.insert(s.spells, { slot = slot, name = name })
        end
    end)
end

function M.grounds(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    local mind     = brain.new()
    local facts    = query.bind(mind, ctx:bot(index))
    local profiles = {
        { name = "mage6", snap = profile("MAGE", 6, 150, 300, nil, { "화염주" }) },
        { name = "poet18", snap = profile("POET", 18, 500, 800, nil, { "공력증강", "화염주", "천공의기원" }) },
        { name = "rogue50", snap = profile("ROGUE", 50, 3000, 1500, "작은물병", { "투명", "비영승보", "삼중공격" }) },
        { name = "warrior63", snap = profile("WARRIOR", 63, 5000, 1500, "작은물병", {}) },
    }
    local ranks    = {}
    local passed   = true
    for _, case in ipairs(profiles) do
        local best       = ground.choose(case.snap, facts)
        local finish     = best ~= nil and best.floors[#best.floors] or nil
        passed           = passed and best ~= nil
        ranks[case.name] = finish ~= nil and finish.need or -1
        log("debug", string.format(
            "[brain:state:grounds] %s bundle=%s start=%s finish=%s floors=%d rate=%.2f margin=%.2f targets=%s", case
            .name,
            best ~= nil and best.name or "nil", best ~= nil and tostring(best.floors[1].map) or "-",
            finish ~= nil and tostring(finish.map) or "-", best ~= nil and #best.floors or 0,
            best ~= nil and best.rate or 0,
            best ~= nil and best.margin or 0, finish ~= nil and table.concat(finish.targets, ",") or ""))
    end
    passed = passed and ranks.rogue50 >= ranks.mage6
    log(passed and "debug" or "fatal", string.format("[brain:state:grounds] %s", passed and "SUCCESS" or "FAILED"))
    return passed
end

function M.field(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    bot:create_item("목도", 1)
    bot:equip(bot:item_slot("목도"))

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, persona = { hunt = 1 } })
    local hunted = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        local _, tasks = history(mind, current)
        local attacked = false
        for _, activity in ipairs(mind:activities(current) or {}) do
            attacked = attacked or activity == "attack"
        end
        return tasks.FIELD ~= nil and attacked
    end)
    local map = id2map(ctx:bot(index):map())
    return finish(ctx, index, mind, "field", hunted, string.format("hunted=%s map=%s", tostring(hunted),
        map ~= nil and map:name() or "?"))
end

function M.caster(ctx, index)
    local bot = ctx:bot(index)
    gear.strip(bot, { "WEAPON" })
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("주술사")
    bot:chat("/레벨바꾸기 10")
    bot = enter(ctx, index)
    if bot == nil then
        return false
    end
    bot:learn_spell("화염주")
    bot:set_max_hp_mp(bot:base_hp(), 3000)
    bot:set_current_hp_mp(bot:base_hp(), 3000)

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, persona = { hunt = 1 } })
    local cast  = false
    local swing = false
    local done  = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        for _, activity in ipairs(mind:activities(current) or {}) do
            cast  = cast or activity == "cast"
            swing = swing or activity == "swing"
        end
        return cast
    end)
    local map   = id2map(ctx:bot(index):map())
    return finish(ctx, index, mind, "caster", done and cast and swing == false, string.format(
        "cast=%s swing=%s map=%s", tostring(cast), tostring(swing), map ~= nil and map:name() or "?"))
end

function M.buffer(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    bot:create_item("목도", 1)
    bot:equip(bot:item_slot("목도"))
    bot:learn_spell("신의축복")
    bot:set_max_hp_mp(bot:base_hp(), 300)
    bot:set_current_hp_mp(bot:base_hp(), 300)

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, persona = { hunt = 1 } })
    local done = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        for _, name in ipairs(mind:observe(current).buffs or {}) do
            if name == "신의축복" then
                return true
            end
        end
        return false
    end)
    local _, tasks = history(mind, ctx:bot(index))
    return finish(ctx, index, mind, "buffer", done and tasks.BUFF ~= nil, string.format("buffed=%s", tostring(done)))
end

function M.striker(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    bot:create_item("목도", 1)
    bot:equip(bot:item_slot("목도"))
    bot:learn_spell("건곤대나이")
    local hp = bot:base_hp() * STRIKER.hp
    bot:set_max_hp_mp(hp, 300)
    bot:set_current_hp_mp(hp, 300)

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1, persona = { hunt = 1 } })
    local cast = false
    local done = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        for _, activity in ipairs(mind:activities(current) or {}) do
            cast = cast or activity == "cast"
        end
        return cast
    end)
    return finish(ctx, index, mind, "striker", done and cast, string.format("cast=%s", tostring(cast)))
end

function M.supplier(ctx, index)
    local bot = ctx:bot(index)
    gear.strip(bot, { "WEAPON" })
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("주술사")
    bot:chat("/레벨바꾸기 10")
    bot:chat(string.format("/금전 %d", SUPPLIER.money))
    bot = enter(ctx, index)
    if bot == nil then
        return false
    end
    bot:learn_spell("화염주")
    bot:set_max_hp_mp(bot:base_hp(), SUPPLIER.mp)
    bot:set_current_hp_mp(bot:base_hp(), SUPPLIER.mp)

    local mind   = brain.new()
    local bought = 0
    local cast   = false
    local swing  = false
    ctx:bot(index):brain(mind, { seed = 1, persona = { hunt = 1 } })
    local done = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        bought = math.max(bought, current:item_count("동동주") + current:item_count("막걸리"))
        for _, activity in ipairs(mind:activities(current) or {}) do
            cast  = cast or activity == "cast"
            swing = swing or activity == "swing"
        end
        return bought > 1 and cast
    end)
    local _, tasks = history(mind, ctx:bot(index))
    return finish(ctx, index, mind, "supplier", done and tasks.SUPPLY ~= nil and swing == false,
        string.format("bought=%d cast=%s swing=%s money=%d", bought, tostring(cast), tostring(swing),
            ctx:bot(index):money()))
end

function M.seller(ctx, index)
    local bot = stage(ctx, index)
    if bot == nil then
        return false
    end
    bot:fill_inventory(SELLER.spare)

    local mind = brain.new()
    ctx:bot(index):brain(mind, { seed = 1 })
    local done = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return current:item_count(SELLER.spare) == 0
    end)
    local _, tasks = history(mind, ctx:bot(index))
    return finish(ctx, index, mind, "seller", done and tasks.SELL ~= nil,
        string.format("sold=%s money=%d", tostring(done), ctx:bot(index):money()))
end

local SLOTS          = { "WEAPON", "ARMOR", "HELMET" }
local PROFILE_TOP    = 5
local PROFILE_REPEAT = 20
local PROFILE_SETTLE = 2000

local function dress(ctx, index, profile)
    local bot = ctx:bot(index)
    gear.strip(bot, SLOTS)
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class(profile.class)
    bot:chat(string.format("/레벨바꾸기 %d", profile.level))
    bot:chat(string.format("/힘바꾸기 %d", profile.str))
    bot:chat(string.format("/지력바꾸기 %d", profile.int))
    bot:chat(string.format("/민첩바꾸기 %d", profile.dex))
    bot:set_max_hp_mp(profile.hp, profile.mp)
    bot:set_current_hp_mp(profile.hp, profile.mp)
    bot:learn_spells(profile.spells)
    for _, slot in ipairs(SLOTS) do
        local name = profile.gear[slot]
        if name ~= nil then
            bot:create_item(name, 1)
            bot:equip(bot:item_slot(name))
        end
    end
    ctx:sleep(PROFILE_SETTLE)
    ctx:bot(index):request(resp.internal_info, protocol.self_info(), function() return true end, TIMEOUT)
    return ctx:bot(index)
end

function M.profiles(ctx, index)
    if stage(ctx, index) == nil then
        return false
    end
    local passed = true
    for _, profile in ipairs(profiles) do
        local bot  = dress(ctx, index, profile)
        local mind = brain.new()
        bot:brain(mind, { seed = 1, persona = { hunt = 1 } })
        expect.within(ctx, TIMEOUT, function()
            return mind:state(ctx:bot(index)) ~= nil
        end)
        local current = ctx:bot(index)
        local snap    = snapshot.take(mind, current)
        local facts   = query.bind(mind, current)
        local ability = score.capability(score.rested(snap), facts)
        local best    = ground.choose(snap, facts)
        local finish  = best ~= nil and best.floors[#best.floors] or nil
        passed        = passed and best ~= nil
        log(best ~= nil and "debug" or "fatal", string.format(
            "[brain:state:profiles] %s lv=%d hp=%d mp=%d phydef=%s dam=%s hit=%s spells=%d dps=%.1f/%.1f burst=%.0f refill=%.1f/%.1f bundle=%s start=%s finish=%s floors=%d rate=%.2f margin=%.2f need=%.0f targets=%s",
            profile.name, snap.self.level, snap.self.max_hp, snap.self.max_mp or 0, tostring(snap.self.phydef),
            tostring(snap.self.dam), tostring(snap.self.hit), #(snap.spells or {}), ability.dps.single,
            ability.dps.melee, ability.burst, ability.refill.hp, ability.refill.mp,
            best ~= nil and best.name or "nil", best ~= nil and tostring(best.floors[1].map) or "-",
            finish ~= nil and tostring(finish.map) or "-", best ~= nil and #best.floors or 0,
            best ~= nil and best.rate or 0, best ~= nil and best.margin or 0, best ~= nil and best.need or 0,
            finish ~= nil and table.concat(finish.targets, ",") or ""))
        local top = {}
        for rank, entry in ipairs(ground.ranked(snap, facts)) do
            if rank > PROFILE_TOP then
                break
            end
            table.insert(top, string.format("%s:%.1f:%s", entry.candidate.bundle.name, entry.rate,
                tostring(ground.reachable(snap, facts, entry.candidate))))
        end
        log("debug", string.format("[brain:state:profiles] %s top=%s", profile.name, table.concat(top, ",")))
        local started = mind:observe(current).now
        for _ = 1, PROFILE_REPEAT do
            decide.choose(snapshot.take(mind, current), facts, nil)
        end
        log("debug", string.format("[brain:state:profiles] %s decide=%.1fms", profile.name,
            (mind:observe(current).now - started) / PROFILE_REPEAT))
        expect.release(ctx, index, mind, TIMEOUT)
    end
    log(passed and "debug" or "fatal", string.format("[brain:state:profiles] %s", passed and "SUCCESS" or "FAILED"))
    return passed
end

return M
