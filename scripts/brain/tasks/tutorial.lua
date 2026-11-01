local protocol = require("integration.protocol")
local resp     = require("integration.response")
local dialog   = require("brain.policies.dialog")
local travel   = require("brain.policies.travel")
local hunt     = require("brain.policies.hunt")
local wait     = require("brain.policies.wait")
local career   = require("brain.policies.job")
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
    dialog.converse(brain, bot, protocol.item_active(slot), answers or {})
    dialog.settled(bot)
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

local function receive(brain, bot)
    local npc = brain:npc(bot, GIVER)
    if npc == nil then
        return false
    end

    dialog.converse(brain, bot, protocol.click(npc.oid), {})
    dialog.settled(bot)
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
    return read(brain, bot, 4) and travel.go(brain, bot, { names = grounds.beginner }) and read(brain, bot, 4) and
        granted(bot, SCROLLS[5])
end

local function prey(brain, bot)
    if read(brain, bot, 5) == false then
        return false
    end

    local counts = {}
    for _, name in ipairs(PREYS) do
        counts[name] = 0
    end
    local hunted = hunt.go(brain, bot, {
        names  = PREYS,
        wants  = function(name)
            return counts[name] ~= nil and counts[name] < PREY_GOAL
        end,
        killed = function(mob)
            counts[mob.name] = counts[mob.name] + 1
        end,
        done   = function()
            for _, count in pairs(counts) do
                if count < PREY_GOAL then
                    return false
                end
            end
            return true
        end,
    })
    return hunted and read(brain, bot, 5) and granted(bot, SCROLLS[6])
end

local function grow(brain, bot)
    if read(brain, bot, 7) == false then
        return false
    end

    local names = {}
    for _, spawn in ipairs(brain:spawns(bot)) do
        names[spawn.name] = true
    end
    local grown = hunt.go(brain, bot, {
        names  = nil,
        wants  = function(name)
            return names[name] ~= nil
        end,
        killed = function()
        end,
        done   = function()
            return bot:level() >= LEVEL_GOAL
        end,
    })
    return grown and read(brain, bot, 7) and granted(bot, SCROLLS[8])
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
    return choice ~= nil and read(brain, bot, 8, { choice }) and bot:has_item_by_name(SCROLLS[8]) == false
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

function M.step(bot)
    local step = 0
    for index, name in ipairs(SCROLLS) do
        if bot:has_item_by_name(name) then
            step = index
        end
    end
    return step
end

function M.wanted(brain, bot, seen)
    return seen.ghost == false and seen.class == "NONE" and (M.step(bot) > 0 or seen.level < job_data.level)
end

function M.run(brain, bot, step)
    local handler = STEPS[step]
    if handler == nil then
        return false
    end
    return handler(brain, bot)
end

return M
