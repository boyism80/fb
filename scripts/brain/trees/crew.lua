local tree = require("brain.tree")
local part = require("brain.tasks.part")

local choose, when, act = tree.select, tree.when, tree.act

local function grouped(snap)
    return #(snap.group or {}) > 0
end

local function worth_leaving(snap, facts)
    return part.value(snap, facts) > 0
end

return choose({
    when(grouped, when(worth_leaving, choose({
        when(part.stricken, act("PART", part.propose, "part_stricken")),
        when(part.finished, act("PART", part.propose, "part_finished")),
    }))),
}, "crew")
