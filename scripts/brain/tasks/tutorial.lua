local protocol = require("integration.protocol")
local resp     = require("integration.response")
local dialog   = require("brain.policies.dialog")
local travel   = require("brain.policies.travel")
local hunt     = require("brain.policies.hunt")
local wait     = require("brain.policies.wait")
local career   = require("brain.policies.job")
local combat   = require("brain.data.combat")
local grounds  = require("brain.data.grounds")
local job_data = require("brain.data.job")

local M = {}

local GIVER      = "낙랑"
local EMPTY_SLOT = 0xFF
local GRANT_WAIT = 3000
local DOTOL      = "도톨"
local DOTOL_USES = 2
local WEAPON     = "초심자의목도"
local QUIZ       = { 3, 3, 3, 1 }
local PREYS      = { "다람쥐", "토끼" }
local PREY_GOAL  = 5
local LEVEL_GOAL = 5
local QUIZ_LEVEL = 3
local SCROLLS    = {
    [1] = "낙랑의두루마리1",
    [2] = "낙랑의두루마리2",
    [3] = "낙랑의두루마리3",
    [4] = "낙랑의두루마리4",
    [5] = "낙랑의두루마리5",
    [6] = "낙랑의두루마리6",
    [7] = "낙랑의두루마리7",
    [8] = "낙랑의두루마리8",
}

local function granted(bot, name)
    if bot:has_item_by_name(name) then
        return true
    end
    wait.reply(bot, resp.item_update, nil, function(packet)
        return packet.name ~= nil and string.find(packet.name, name, 1, true) ~= nil
    end, GRANT_WAIT)
    return bot:has_item_by_name(name)
end

local function read(brain, bot, step, answers)
    local slot = bot:item_slot(SCROLLS[step])
    if slot == EMPTY_SLOT then
        return false
    end
    brain:talk(bot, protocol.item_active(slot), answers or {})
    return true
end

local function use(brain, bot, name, count)
    for _ = 1, count do
        if brain:active(bot) == false or dialog.consume(bot, name) == false then
            return false
        end
    end
    return true
end

local function approach(brain, bot)
    local spots = brain:npcs(bot, { GIVER })
    local maps  = {}
    for _, spot in ipairs(spots) do
        table.insert(maps, spot.map)
    end
    if #maps == 0 then
        return false, "NO_NPC"
    end
    local arrived, event = travel.go(brain, bot, { maps = maps })
    if arrived == false then
        return false, travel.yielded(event) and "ENGAGED" or "UNREACHABLE"
    end
    local range = math.min(combat.sight.x, combat.sight.y)
    for _, spot in ipairs(spots) do
        if spot.map == bot:map() then
            local near, moved = travel.go(brain, bot, { map = spot.map, x = spot.x, y = spot.y, range = range })
            if near then
                return true
            elseif travel.yielded(moved) then
                return false, "ENGAGED"
            end
        end
    end
    return false, "UNREACHABLE"
end

local function receive(brain, bot)
    local npc = brain:npc(bot, GIVER)
    if npc == nil then
        local reached, reason = approach(brain, bot)
        if reached == false then
            return reason == "ENGAGED", reason
        end
        npc = brain:npc(bot, GIVER)
    end
    if npc == nil then
        return false, "NO_NPC"
    end

    brain:talk(bot, protocol.click(npc.oid), {})
    return granted(bot, SCROLLS[1])
end

local function inn(brain, bot)
    return read(brain, bot, 1) and granted(bot, SCROLLS[2])
end

local function dotol(brain, bot)
    return read(brain, bot, 2) and use(brain, bot, DOTOL, DOTOL_USES) and read(brain, bot, 2) and
        granted(bot, SCROLLS[3])
end

local function weapon(brain, bot)
    return read(brain, bot, 3) and use(brain, bot, WEAPON, 1) and read(brain, bot, 3) and
        granted(bot, SCROLLS[4])
end

local function ground(brain, bot)
    if read(brain, bot, 4) == false then
        return false
    end
    local arrived, event = travel.go(brain, bot, { names = grounds.beginner })
    if arrived == false then
        return travel.yielded(event), event and event.type
    end
    return read(brain, bot, 4) and granted(bot, SCROLLS[5])
end

local function hunting(brain, bot, step, destination, goal)
    if brain:recall(bot, "tutorial") ~= step then
        if read(brain, bot, step) == false then
            return false
        end
        brain:remember(bot, "tutorial", step)
    end
    local arrived, event = travel.go(brain, bot, destination)
    if arrived == false then
        return travel.yielded(event), event and event.type
    end
    local result, reason = hunt.step(brain, bot, goal)
    if result == "PROGRESS" then
        return true
    elseif result == "FAILED" then
        return false, reason
    else
        brain:remember(bot, "tutorial", nil)
        return read(brain, bot, step) and granted(bot, SCROLLS[step + 1])
    end
end

local function prey(brain, bot)
    local wanted = {}
    for _, name in ipairs(PREYS) do
        wanted[name] = true
    end
    return hunting(brain, bot, 5, { maps = hunt.beginner(brain, bot, PREYS) }, {
        key    = "prey",
        names  = PREYS,
        wants  = function(state, name)
            return wanted[name] == true and (state.counts[name] or 0) < PREY_GOAL
        end,
        killed = function(state, mob)
            state.counts[mob.name] = (state.counts[mob.name] or 0) + 1
        end,
        done   = function(state)
            for _, name in ipairs(PREYS) do
                if (state.counts[name] or 0) < PREY_GOAL then
                    return false
                end
            end
            return true
        end,
    })
end

local function grow(brain, bot)
    local names = {}
    for _, spawn in ipairs(brain:spawns(bot)) do
        names[spawn.name] = true
    end
    return hunting(brain, bot, 7, { names = grounds.beginner }, {
        key    = "grow",
        names  = nil,
        wants  = function(_, name)
            return names[name] ~= nil
        end,
        killed = function()
        end,
        done   = function()
            return bot:level() >= LEVEL_GOAL
        end,
    })
end

local function quiz(brain, bot)
    if bot:level() < QUIZ_LEVEL then
        return false
    end
    return read(brain, bot, 6, QUIZ) and granted(bot, SCROLLS[7])
end

local function job(brain, bot)
    local chosen = career.choose(brain, bot)
    local choice = nil
    for index, name in ipairs(job_data.scroll) do
        if name == chosen then
            choice = index
        end
    end
    if choice == nil then
        return false, "NO_JOB"
    elseif read(brain, bot, 8, { choice }) == false then
        return false, "NO_SCROLL"
    elseif bot:has_item_by_name(SCROLLS[8]) then
        return false, "UNCHANGED"
    else
        return true
    end
end

local STEPS = {
    [0] = receive,
    [1] = inn,
    [2] = dotol,
    [3] = weapon,
    [4] = ground,
    [5] = prey,
    [6] = quiz,
    [7] = grow,
    [8] = job,
}

function M.step(snap)
    local held = {}
    for _, item in ipairs(snap.bag.items) do
        held[item.name] = true
    end
    local step = 0
    for index, name in ipairs(SCROLLS) do
        if held[name] then
            step = index
        end
    end
    return step
end

function M.wanted(snap)
    return snap.self.ghost == false and snap.self.class == "NONE" and
        (M.step(snap) > 0 or snap.self.level < job_data.level)
end

function M.run(brain, bot, step)
    local handler = STEPS[step]
    if handler == nil then
        return false
    end
    return handler(brain, bot)
end

return M
