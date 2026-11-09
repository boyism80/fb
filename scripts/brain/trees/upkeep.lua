local combat  = require("brain.policies.combat")
local tree    = require("brain.tree")
local buff    = require("brain.tasks.buff")
local equip   = require("brain.tasks.equip")
local loot    = require("brain.tasks.loot")
local recover = require("brain.trees.recover")
local support = require("brain.trees.support")

local choose, when, act = tree.select, tree.when, tree.act

local M = {}

local function alive(snap)
    return snap.self.ghost == false
end

local function has_class(snap)
    return snap.self.class ~= "NONE"
end

local function has_buff(snap, facts)
    return buff.best(snap, facts) ~= nil
end

local function buff_pays(snap, facts)
    return buff.value(snap, facts) >= 0
end

local function has_upgrade(snap, facts)
    return equip.best(snap, facts) ~= nil
end

local function upgrade_pays(snap, facts)
    return equip.value(snap, facts) >= 0
end

local function has_pile(snap, facts)
    return loot.pile(snap, facts) ~= nil
end

local function pile_pays(snap, facts)
    return loot.value(snap, facts) >= 0
end

local pickup = choose({
    when(loot.on_pile, act("LOOT", loot.pick, "pick_up")),
    act("LOOT", loot.walk, "to_pile"),
}, "pickup")

M.loot = when(alive, when(has_pile, pickup), "loot")

M.tree = when(alive, choose({
    recover,
    when(combat.can_cast, choose({
        when(buff.hunting, when(has_buff, when(buff_pays, act("BUFF", buff.cast, "self_buff")))),
        support.care,
    })),
    when(has_pile, when(pile_pays, pickup)),
    when(has_class, when(has_upgrade, when(upgrade_pays, act("EQUIP", equip.propose, "equip")))),
}), "upkeep")

return M
