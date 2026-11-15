local tree = require("brain.tree")
local part = require("brain.tasks.part")

local choose, when, act = tree.select, tree.when, tree.act

local function grouped(snap)
    return #(snap.group or {}) > 0
end

return choose({
    when(grouped, choose({
        when(part.finished, act("PART", part.propose, "part_finished")),
        when(part.lapped, choose({
            when(part.dissolved, act("PART", part.propose, "part_dissolved")),
            when(part.unwanted, act("PART", part.propose, "part_unwanted")),
        })),
    })),
}, "crew")
