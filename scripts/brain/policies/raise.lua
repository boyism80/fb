local data   = require("brain.data.combat")
local magic  = require("brain.data.spells")
local combat = require("brain.policies.combat")
local danger = require("brain.policies.danger")
local party  = require("brain.policies.party")
local value  = require("brain.policies.value")

local M = {}

local function known(member)
    for _, learned in ipairs(member.spells or {}) do
        local spell = magic.spells[learned.name]
        if spell ~= nil and spell.role == "REVIVE" then
            return learned, spell
        end
    end
    return nil, nil
end

-- A crewmate raised where it fell (부활) is back at once beside its own pile: what it saves is the walk to the shrine
-- and back for it and for the crew that hunts without it meanwhile (user 11-15: raise on the spot). Raised it has 50
-- hp on the tile it fell on, so the mobs beside that tile weigh as its death again within two casts, as a heal weighs
-- the death it keeps off (aid); with mobs on its corpse that death outweighs the trip and the poet waits for the tile
-- to clear. The death itself is priced as before (user 11-15: the death cost is not lowered).
local function worth(snap, query, member, at)
    local _, spell = known(snap)
    local rates    = value.baseline(snap, query)
    local seconds  = danger.revival(member, query, member.world.map)
    local mate     = { self = member.self, world = { map = snap.world.map, x = at.x, y = at.y } }
    local blows    = combat.pressure(snap, mate, 2 * combat.cast(snap))
    local risk     = blows > 0 and math.exp(-data.penalty.revive_hp / blows) or 0
    return party.alone(snap, member, seconds, rates) + seconds * rates.value -
        (spell.mp or 0) * value.spend(snap, query, nil) - risk * party.death(snap, query, member, rates)
end

function M.cast(snap, query)
    local learned, spell = known(snap)
    if learned == nil or snap.self.ghost or combat.can_cast(snap) == false or (snap.self.mp or 0) < (spell.mp or 0) then
        return nil
    end
    local best, found = nil, nil
    for _, member in ipairs(party.members(snap)) do
        local ally = member.self.ghost and query.ally(member.self.name) or nil
        if ally ~= nil and ally.x ~= nil and ally.oid ~= nil then
            local gain = worth(snap, query, member, ally)
            if best == nil or gain > best then
                best  = gain
                found = {
                    kind = ACTION.CAST,
                    slot = learned.slot,
                    oid = ally.oid,
                    x = ally.x,
                    y = ally.y,
                    raise = member
                        .self.name
                }
            end
        end
    end
    return found
end

function M.raised(snap, query, action)
    for _, member in ipairs(party.members(snap)) do
        if member.self.name == action.raise then
            return worth(snap, query, member, query.ally(member.self.name))
        end
    end
    return nil
end

-- The answer a poet posts for a fallen crewmate: a promise with the time it takes to reach it, or a refusal when it
-- cannot raise it (another floor, no way there, no mana and none to come). The crew holds its ground on a promise and
-- leaves on a refusal (abandoned), and the fallen waits for the promise as long as it beats the shrine (wait).
function M.promise(snap, query)
    local learned, spell = known(snap)
    if learned == nil or snap.self.ghost then
        return nil
    end
    for _, member in ipairs(party.members(snap)) do
        local fallen = member.fallen
        if member.self.ghost and fallen ~= nil then
            local mana  = (snap.self.mp or 0) >= (spell.mp or 0) or require("brain.policies.recovery").sustains(snap)
            local able  = mana and fallen.map == snap.world.map and member.world.x ~= nil and snap.world.x ~= nil
            local steps = able and query ~= nil and query.steps ~= nil and
                query.steps(snap.world.map, snap.world.x, snap.world.y,
                    { { x = member.world.x, y = member.world.y, range = 1 } })[1] or nil
            able        = able and (steps == nil or (type(steps) == "number" and steps < math.huge))
            return {
                name    = member.self.name,
                at      = snap.now,
                eta     = able and type(steps) == "number" and steps * (snap.pace.step or data.walk) + combat.cast(snap) or
                    nil,
                refused = able == false or nil,
            }
        end
    end
    return nil
end

-- A fallen crewmate sends the crew off its ground only when no poet of the crew will raise it: with one alive that
-- knows 부활 the crew holds its ground until that poet refuses, and holds while its answer has not come, as the
-- profile comes two seconds late (user 11-15). Left as it was, every death sent the crew running and the corpse was
-- walked back to from the shrine.
function M.abandoned(snap, member, fallen)
    local crew = { snap }
    for _, other in ipairs(party.members(snap)) do
        table.insert(crew, other)
    end
    for _, other in ipairs(crew) do
        if other.self.name ~= member.self.name and other.self.ghost == false and known(other) ~= nil then
            local raising = other == snap and M.promise(snap, nil) or other.raising
            if raising == nil or raising.name ~= member.self.name or raising.at < fallen.at or raising.refused ~= true then
                return false
            end
        end
    end
    return true
end

-- Fallen, I wait on my corpse for the poet that promised to raise me while the wait so far and the time it says it
-- needs stay under the walk to the shrine and back: a poet that does not come closer keeps its time while my wait
-- grows, so the wait ends by itself and I walk to the shrine.
function M.wait(snap, query)
    local death = snap.record.death
    if snap.self.ghost == false or death == nil then
        return nil
    end
    local waited = death.age or 0
    local died   = snap.now - waited * 1000
    local shrine = danger.revival(snap, query)
    for _, member in ipairs(party.members(snap)) do
        local raising = member.raising
        if member.self.ghost == false and raising ~= nil and raising.name == snap.self.name and raising.refused ~= true and
            raising.at >= died and raising.eta ~= nil and waited + raising.eta / 1000 < shrine then
            return { kind = ACTION.HOLD }
        end
    end
    return nil
end

-- A poet fallen itself raises itself on its corpse when that beats the shrine (the server lets a ghost cast 부활 on
-- itself only), weighed as it would raise a crewmate.
function M.self_cast(snap, query)
    local learned, spell = known(snap)
    if learned == nil or snap.self.ghost == false or (snap.self.mp or 0) < (spell.mp or 0) or snap.world.x == nil then
        return nil
    end
    return worth(snap, query, snap, snap.world) > 0 and { kind = ACTION.CAST, slot = learned.slot } or nil
end

return M
