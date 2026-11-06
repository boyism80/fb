local buyers   = require("brain.data.buyers")
local combat   = require("brain.data.combat")
local learning = require("brain.data.learning")
local outfits  = require("brain.data.outfits")
local shops    = require("brain.data.shops")

local M = {}

local function present(episode, names)
    local spots = episode:main(function()
        return episode.mind:npcs(nil, names)
    end)
    for _, spot in ipairs(spots or {}) do
        if spot.map == episode.active.map.id and math.abs(spot.x - episode.me.x) <= combat.sight.x and
            math.abs(spot.y - episode.me.y) <= combat.sight.y then
            return spot
        end
    end
    return nil
end

local function held(bag, name)
    local total = 0
    for _, item in ipairs(bag.items) do
        if item.name == name then
            total = total + item.count
        end
    end
    return total
end

local function space(bag, name, capacity)
    local free = 0
    for _, item in ipairs(bag.items) do
        if item.name == name then
            free = free + math.max(0, capacity - item.count)
            if capacity > 1 then
                return free
            end
        end
    end
    local slots = math.max(0, (bag.capacity or 0) - (bag.used or 0))
    return free + (capacity > 1 and math.min(slots, 1) or slots) * capacity
end

local function add(bag, name, count, capacity, kind)
    for _, item in ipairs(bag.items) do
        if item.name == name and count > 0 then
            local moved = math.min(count, capacity - item.count)
            item.count  = item.count + moved
            count       = count - moved
        end
    end
    while count > 0 do
        local moved = math.min(count, capacity)
        bag.used    = (bag.used or 0) + 1
        table.insert(bag.items, { slot = #bag.items + 1, name = name, count = moved, type = kind or "CONSUME" })
        count = count - moved
    end
end

local function remove(bag, name)
    local kept = {}
    for _, item in ipairs(bag.items) do
        if item.name == name then
            bag.used = math.max(0, (bag.used or 0) - 1)
        else
            table.insert(kept, item)
        end
    end
    bag.items = kept
end

function M.gather(bag, name, count, capacity)
    local held = false
    for _, item in ipairs(bag.items) do
        held = held or item.name == name
    end
    if capacity > 1 and held then
        local room = 0
        for _, item in ipairs(bag.items) do
            if item.name == name then
                room = room + math.max(0, capacity - item.count)
            end
        end
        if room < count then
            return false
        end
    elseif space(bag, name, capacity) < count then
        return false
    end
    add(bag, name, count, capacity)
    return true
end

function M.buy(episode, name, count)
    local shop = shops[name] or outfits[name]
    if shop == nil or present(episode, shop.npcs) == nil then
        return false, "NO_NPC"
    end
    local capacity = shop.capacity or 1
    if capacity <= 1 then
        count = 1
    end
    local price = shop.price * count
    if price > episode.money then
        return false, "NO_MONEY"
    end
    if count > space(episode.bag, name, capacity) then
        return false, "FULL"
    end
    episode.money = episode.money - price
    if (shop.uses or 1) > 1 then
        table.insert(episode.bag.items, { slot = #episode.bag.items + 1, name = name, count = shop.uses })
        episode.bag.used = (episode.bag.used or 0) + 1
        return true
    end
    add(episode.bag, name, count, capacity, outfits[name] ~= nil and outfits[name].type or nil)
    return true
end

function M.sell(episode, name)
    local buyer = buyers[name]
    if buyer == nil or present(episode, buyer.npcs) == nil then
        return false, "NO_NPC"
    end
    local count = held(episode.bag, name)
    if count == 0 then
        return false, "NO_ITEM"
    end
    episode.money = episode.money + buyer.price * count
    remove(episode.bag, name)
    return true
end

function M.appreciate(episode)
    if present(episode, combat.shrine.npcs) == nil then
        return false, "NO_NPC"
    end
    if episode.me.hp <= combat.shrine.limit then
        episode.me.hp = episode.me.max_hp
    end
    return true
end

function M.say(episode, action)
    local text        = action.text or ""
    local name, count = text:match("^(%S+) (%d+)개 줘$")
    if name ~= nil then
        return M.buy(episode, name, tonumber(count))
    end
    name = text:match("^(%S+) 다 판다$")
    if name ~= nil then
        return M.sell(episode, name)
    end
    if text == combat.shrine.words then
        return M.appreciate(episode)
    end
    return false, "UNSUPPORTED"
end

local function lesson(class, name)
    for _, entry in ipairs(learning[class] or {}) do
        for _, spell in ipairs(entry.names) do
            if spell == name then
                return entry
            end
        end
    end
    return nil
end

function M.learn(episode, action)
    if action.npc ~= nil and present(episode, { action.npc }) == nil then
        return false, "NO_NPC"
    end
    local entry = lesson(episode.base.self.class, action.spell)
    if entry == nil or entry.level > episode.base.self.level or entry.money > episode.money then
        return false, "REFUSED"
    end
    for item, count in pairs(entry.materials or {}) do
        if held(episode.bag, item) < count then
            return false, "REFUSED"
        end
    end
    for _, known in ipairs(episode.spells) do
        if known.name == action.spell then
            return false, "UNCHANGED"
        end
    end
    episode.money = episode.money - entry.money
    for item, count in pairs(entry.materials or {}) do
        local left = count
        for _, stack in ipairs(episode.bag.items) do
            if stack.name == item and left > 0 then
                local taken = math.min(left, stack.count)
                stack.count = stack.count - taken
                left        = left - taken
            end
        end
    end
    local learned = {}
    for _, known in ipairs(episode.spells) do
        table.insert(learned, known)
    end
    table.insert(learned, { slot = #learned + 1, name = action.spell })
    episode.spells = learned
    return true
end

function M.talk(episode, action)
    if action.spell ~= nil then
        return M.learn(episode, action)
    end
    return false, "UNSUPPORTED"
end

return M
