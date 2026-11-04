local M = {}

local LIMIT = 80

local fights  = {}
local grounds = {}

local function seen(brain, bot)
    local view = brain:observe(bot)
    return {
        now = view.now,
        map = view.map,
        hp = view.hp,
        mp = view.mp,
        exp = view.exp,
        money = view.money,
        level = view.level,
        class = view.class,
        ghost = view.ghost,
    }
end

local function close(brain, bot, fight, result, view)
    fights[bot:name()] = nil
    brain:trace(bot, {
        kind = "fight",
        result = result,
        task = fight.task,
        oid = fight.oid,
        mob = fight.mob,
        id = fight.id,
        map = fight.map,
        level = fight.level,
        class = fight.class,
        forecast = fight.forecast,
        actual = {
            time = view.now - fight.started,
            hp = view.hp - fight.hp,
            mp = view.mp - fight.mp,
            exp = view.exp - fight.exp,
            low = fight.low,
            uses = fight.uses,
            events = fight.events,
            steps = fight.steps,
        },
    })
end

function M.engage(brain, bot, oid, action, event)
    local name  = bot:name()
    local view  = seen(brain, bot)
    local fight = fights[name]
    if fight ~= nil and fight.oid ~= oid then
        close(brain, bot, fight, "SWITCHED", view)
        fight = nil
    end
    if fight == nil then
        local mob = nil
        if action.mob == nil then
            for _, entry in ipairs(brain:mobs(bot)) do
                if entry.oid == oid then
                    mob = entry
                end
            end
        end
        fight = {
            oid = oid,
            mob = action.mob or (mob ~= nil and mob.name or nil),
            id = action.mob_id or (mob ~= nil and mob.id or nil),
            task = action.task,
            map = view.map,
            level = view.level,
            class = view.class,
            started = view.now,
            last = view.now,
            hp = view.hp,
            mp = view.mp,
            exp = view.exp,
            low = view.hp,
            forecast = action.forecast,
            uses = {},
            events = {},
            steps = {},
        }
        fights[name] = fight
    end
    local key           = action.key or "SWING"
    fight.uses[key]     = (fight.uses[key] or 0) + 1
    fight.events[event] = (fight.events[event] or 0) + 1
    fight.low           = math.min(fight.low, view.hp)
    if #fight.steps < LIMIT then
        local position = bot:position()
        local near     = 0
        for _, entry in ipairs(brain:mobs(bot)) do
            if entry.x ~= nil and position ~= nil and math.abs(entry.x - position[1]) + math.abs(entry.y - position[2]) == 1 then
                near = near + 1
            end
        end
        table.insert(fight.steps, {
            at = view.now - fight.started,
            key = key,
            event = event,
            hp = view.hp,
            mp = view.mp,
            near = near
        })
    end
    fight.last = view.now
    if event == "KILLED" then
        close(brain, bot, fight, "KILLED", seen(brain, bot))
    end
end

function M.act(brain, bot, action, done, reason)
    local name  = bot:name()
    local fight = fights[name]
    local view  = nil
    if fight ~= nil and action.kind ~= ACTION.ATTACK then
        view            = seen(brain, bot)
        local key       = "ACT:" ..
            tostring(action.task or action.kind) .. (action.item ~= nil and (":" .. action.item) or "")
        fight.uses[key] = (fight.uses[key] or 0) + 1
        if #fight.steps < LIMIT then
            table.insert(fight.steps,
                {
                    at = view.now - fight.started,
                    key = key,
                    event = reason or (done and "DONE" or "FAILED"),
                    hp = view.hp,
                    mp = view.mp
                })
        end
    end
    if fight ~= nil then
        view = view or seen(brain, bot)
        if view.ghost then
            close(brain, bot, fight, "DIED", view)
        elseif view.map ~= fight.map then
            close(brain, bot, fight, "LEFT", view)
        end
    end
    if done == false then
        brain:trace(bot, {
            kind = "action",
            task = action.task,
            action = action.kind,
            reason = reason,
            npc = action.npc,
            spell = action.spell,
            item = action.item,
            map = (view or seen(brain, bot)).map,
        })
    end
    local occupy = action.occupy
    if occupy ~= nil and occupy.map ~= nil and grounds[name] ~= occupy.map then
        grounds[name] = occupy.map
        view = view or seen(brain, bot)
        brain:trace(bot, {
            kind = "ground",
            map = occupy.map,
            level = view.level,
            class = view.class,
            exp = view.exp,
            money = view.money,
            forecast = {
                kills = occupy.rate,
                exp = occupy.exp,
                deaths = occupy.deaths,
                margin = occupy.margin,
                wear = occupy.wear,
                extra = occupy.extra,
                name = occupy.name,
                crew = occupy.crew
            },
        })
    end
end

function M.decide(brain, bot, snap, values, picked, current)
    brain:trace(bot, {
        kind = "decide",
        now = snap.now,
        map = snap.world.map,
        x = snap.world.x,
        y = snap.world.y,
        hp = snap.self.hp,
        mp = snap.self.mp or 0,
        current = current,
        picked = picked,
        values = values,
    })
end

return M
