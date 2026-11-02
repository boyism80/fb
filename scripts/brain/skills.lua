local protocol = require("integration.protocol")
local resp     = require("integration.response")
local dialog   = require("brain.policies.dialog")
local travel   = require("brain.policies.travel")
local wait     = require("brain.policies.wait")

local M = {}

local EMPTY_SLOT = 0xFF
local WEAR_WAIT  = 3000

local function synced(bot)
    wait.reply(bot, resp.internal_info, protocol.self_info(), function() return true end, WEAR_WAIT)
end

local function carried(brain, bot)
    local total = 0
    for _, item in ipairs(brain:inventory(bot)) do
        total = total + item.count
    end
    return total
end

local function settled(brain, bot, expect)
    local seen = brain:observe(bot)
    for key, value in pairs(expect or {}) do
        if seen[key] ~= value then
            return false, "UNCHANGED"
        end
    end
    return true
end

local SKILLS = {
    [ACTION.MOVE]   = function(brain, bot, action)
        local arrived, event = travel.go(brain, bot, action.dest)
        return arrived, event and event.type
    end,
    [ACTION.ATTACK] = function(brain, bot, action)
        local event = brain:attack(bot, action.oid, action.tactic or {})
        return event.type == "KILLED", event.type
    end,
    [ACTION.TALK]   = function(brain, bot, action)
        local npc = action.npc ~= nil and brain:npc(bot, action.npc) or nil
        if action.npc ~= nil and npc == nil then
            return false, "NO_NPC"
        end
        local opening = npc ~= nil and protocol.click(npc.oid) or nil
        local talked  = brain:talk(bot, opening, action.answers or {})
        if talked ~= "DONE" and talked ~= "UNANSWERED" then
            return false, talked
        end
        return settled(brain, bot, action.expect)
    end,
    [ACTION.READ]   = function(brain, bot, action)
        local slot = bot:item_slot(action.item)
        if slot == EMPTY_SLOT then
            return false, "NO_ITEM"
        end
        local talked = brain:talk(bot, protocol.item_active(slot), action.answers or {})
        if talked ~= "DONE" and talked ~= "UNANSWERED" then
            return false, talked
        end
        return settled(brain, bot, action.expect)
    end,
    [ACTION.USE]    = function(brain, bot, action)
        for _ = 1, action.count or 1 do
            if brain:active(bot) == false or dialog.consume(bot, action.item) == false then
                return false, "NO_ITEM"
            end
        end
        return true
    end,
    [ACTION.PICK]   = function(brain, bot, action)
        local money  = bot:money()
        local before = carried(brain, bot)
        for _ = 1, #(action.oids or { 0 }) do
            if brain:active(bot) == false then
                return false, "INTERRUPTED"
            end
            brain:loot(bot)
        end
        if bot:money() == money and carried(brain, bot) == before then
            return false, "UNCHANGED"
        end
        return true
    end,
    [ACTION.FORGET] = function(brain, bot, action)
        if action.key ~= nil then
            brain:remember(bot, action.key, nil)
        else
            brain:bury(bot)
        end
        return true
    end,
    [ACTION.EQUIP]  = function(brain, bot, action)
        local slot  = bot:item_slot(action.item)
        local count = bot:item_count(action.item)
        if slot == EMPTY_SLOT then
            return false, "NO_ITEM"
        end
        local worn = wait.reply(bot, resp.message, protocol.item_active(slot), function(packet)
            return packet.type == "STATE"
        end, WEAR_WAIT)
        synced(bot)
        if worn == nil or bot:item_count(action.item) >= count then
            return false, "REFUSED"
        end
        return true
    end,
    [ACTION.CAST]   = function(brain, bot, action)
        local hp, mp = bot:hp(), bot:mp()
        if brain:cast(bot, action.slot, action.oid, action.x, action.y) == false then
            return false, "NO_SPELL"
        end
        synced(bot)
        if action.buff ~= nil then
            for _, name in ipairs(brain:observe(bot).buffs or {}) do
                if name == action.buff then
                    return true
                end
            end
            return false, "UNCHANGED"
        elseif bot:hp() == hp and bot:mp() == mp then
            return false, "UNCHANGED"
        end
        return true
    end,
    [ACTION.SAY]    = function(brain, bot, action)
        local held = action.gain ~= nil and bot:item_count(action.gain) or 0
        local sold = action.lose ~= nil and bot:item_count(action.lose) or 0
        bot:chat(action.text)
        synced(bot)
        if action.gain ~= nil and bot:item_count(action.gain) <= held then
            return false, "UNCHANGED"
        elseif action.lose ~= nil and bot:item_count(action.lose) >= sold then
            return false, "UNCHANGED"
        end
        return settled(brain, bot, action.expect)
    end,
}

function M.run(brain, bot, action)
    local skill = SKILLS[action.kind]
    if skill == nil then
        return false, "UNKNOWN_ACTION"
    end
    if action.forget then
        brain:bury(bot)
    end
    if action.occupy ~= nil then
        brain:occupy(bot, action.occupy.map, action.occupy.rate)
    end
    if action.peril ~= nil then
        local perils = brain:recall(bot, "perils") or {}
        perils[tostring(action.peril.map)] = {
            mob = action.peril.mob,
            stats = action.peril.stats,
            at = brain:observe(bot).now
        }
        brain:remember(bot, "perils", perils)
    end
    local done, reason = skill(brain, bot, action)
    if done and action.clear ~= nil and brain:active(bot) then
        brain:remember(bot, action.clear, nil)
    end
    if done and action.tally ~= nil and brain:active(bot) then
        local record = brain:recall(bot, action.tally)
        if record ~= nil then
            record.killed = (record.killed or 0) + 1
            if record.killed >= (record.count or 1) then
                brain:remember(bot, action.tally, nil)
            else
                brain:remember(bot, action.tally, record)
            end
        end
    end
    return done, reason
end

return M
