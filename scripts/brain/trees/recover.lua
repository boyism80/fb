local tree    = require("brain.tree")
local recover = require("brain.tasks.recover")

local choose, when, act = tree.select, tree.when, tree.act

local function forget()
    return { kind = ACTION.FORGET }
end

local function wear_spare(snap)
    local item = recover.spare(snap)
    return item ~= nil and { kind = ACTION.EQUIP, item = item } or nil
end

local function worthless(snap, facts)
    return recover.exhausted(snap) and forget() or nil
end

local function go_back(snap, facts)
    local death = snap.record.death
    if snap.world.map == death.map and snap.world.x == death.x and snap.world.y == death.y then
        return nil
    end
    local action = { kind = ACTION.MOVE, dest = { map = death.map, x = death.x, y = death.y, range = 0 } }
    return recover.profit(snap, facts, action) > 0 and action or forget()
end

local function pick_up(snap, facts)
    local found = recover.piles(snap)
    if #found == 0 then
        return nil
    end
    local action = { kind = ACTION.PICK, oids = found }
    return recover.profit(snap, facts, action) > 0 and action or forget()
end

return when(function(snap)
    return snap.self.ghost == false and snap.record.death ~= nil
end, choose({
    act("RECOVER", wear_spare, "wear_spare"),
    act("RECOVER", worthless, "give_up"),
    act("RECOVER", go_back, "go_back"),
    act("RECOVER", pick_up, "pick_up"),
    act("RECOVER", forget, "nothing_left"),
}), "recover")
