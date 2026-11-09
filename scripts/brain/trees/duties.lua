local tree     = require("brain.tree")
local order    = require("brain.tasks.order")
local tutorial = require("brain.tasks.tutorial")
local job      = require("brain.tasks.job")

local choose, when, act, task = tree.select, tree.when, tree.act, tree.task

local function alive(snap)
    return snap.self.ghost == false
end

local function has_order(snap)
    return snap.record.memory.order ~= nil
end

local function not_recovering(snap, facts)
    return order.recovering(snap, facts) == false
end

local function patient(snap, facts)
    local wait = order.wait_time(snap, facts)
    if wait > 0 then
        tree.postpone(snap, wait * 1000)
        return false
    end
    return true
end

local function worthwhile(snap, facts)
    return order.value(snap, facts) >= 0
end

local function order_is(kind)
    return function(snap)
        return snap.record.memory.order.task == kind
    end
end

return when(alive, choose({
    when(has_order, when(not_recovering, when(patient, when(worthwhile, choose({
        when(order_is("GO"), act("ORDER", order.go, "order_go")),
        when(order_is("HUNT"), act("ORDER", order.hunt, "order_hunt")),
        act("ORDER", order.forget, "order_unknown"),
    }))))),
    task("TUTORIAL"),
    task("JOB"),
}), "duties")
