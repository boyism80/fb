local data    = require("brain.data.combat")
local battle  = require("brain.policies.battle")
local combat  = require("brain.policies.combat")
local danger  = require("brain.policies.danger")
local failure = require("brain.policies.failure")
local party   = require("brain.policies.party")
local score   = require("brain.policies.score")
local aid     = require("brain.tasks.aid")
local field   = require("brain.tasks.field")
local value   = require("brain.policies.value")

local M = {}

local function attacker(snap, query)
    return battle.target(snap, query, function(mob)
        return battle.attacking(snap, mob) and battle.struck(snap, mob)
    end)
end

function M.wanted(snap, query)
    return snap.self.ghost == false and attacker(snap, query) ~= nil
end

function M.propose(snap, query)
    return battle.propose(attacker(snap, query))
end

local function barring(snap, query)
    return battle.target(snap, query, function(mob)
        return math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1
    end)
end

function M.walled_in(snap)
    local blocked = snap.record.memory.blocked
    return snap.self.ghost == false and type(blocked) == "table" and blocked.map == snap.world.map and
        snap.now - blocked.at < (snap.pace.swing or data.swing)
end

function M.barred(snap, query)
    return barring(snap, query) ~= nil
end

-- Clearing my way is a strike like any other: walled in beside a warrior that turns hp into damage, a poet cast
-- 화염주 to get out, spent the heal its mana was worth and drew the mob onto itself (261 of the 401 offline 화염주
-- of poets with 건곤대나이 warriors, run 21:08). It heals and is freed by its crew instead (hunt free_member).
function M.clear_path(snap, query)
    local found = barring(snap, query)
    if aid.outweighs(snap, query, found) == false then
        return nil
    end
    return battle.propose(found)
end

local function passing(snap, query, mob, fight)
    local plan = party.plan(snap, query)
    if plan == nil or failure.remaining(snap, "FIELD") ~= 0 or field.wanted(snap, query) == false then
        return false
    end
    local blocked = snap.record.memory.blocked
    if type(blocked) == "table" and snap.now - blocked.at < fight.time then
        return false
    end
    if plan.gate == snap.world.map then
        return false
    end
    for _, floor in ipairs(plan.floors) do
        if floor.map == snap.world.map then
            return false
        end
    end
    return fight.time > (mob.speed or 0) and combat.gain(mob, snap) / math.max(fight.time, 1) * 1000 < plan.rate
end

local function backed(snap, query)
    local blow = score.capability(snap, query).blow
    local mine = math.max(blow.adjacent, blow.ranged)
    for _, member in ipairs(party.members(snap)) do
        if member.self.ghost == false and member.world.map == snap.world.map and member.world.x ~= nil and
            math.abs(member.world.x - snap.world.x) <= data.sight.x and
            math.abs(member.world.y - snap.world.y) <= data.sight.y then
            local theirs = score.capability(member, query).blow
            if math.max(theirs.adjacent, theirs.ranged) > mine then
                return true
            end
        end
    end
    return false
end

function M.value(snap, query)
    local found = attacker(snap, query)
    local worth = value.baseline(snap, query)
    local fight = combat.outcome(snap, query, found.mob)
    if passing(snap, query, found.mob, fight) then
        return -1
    end
    -- Striking back only removes the danger of an attacker it ends within the window, so the danger counts in the
    -- share of the fight one cast takes. Counted in full, a poet beside a 18000 hp skeleton cast 화염주 (320) over
    -- healing the warrior whenever it was hit (online group, 14:45, 36 화염주 to 28 heals).
    -- A stronger crewmate fighting beside me kills the attacker anyway, so the same holds with no crew work to
    -- do: counted as a lone 52s fight to the death, the strike was worth the whole kill plus a sure death and a
    -- poet cast 화염주 whenever its warrior was full (online group, 16:08, 66 of 397 weighings). Stronger means
    -- the harder single blow, hp strikes included, whether it fights this moment or rests: by sustained spell
    -- damage a poet refilling with 공력증강 outranked its warrior's 건곤대나이, a resting warrior counted as no
    -- help, and the poet struck back for the whole kill (online group, 05:57, 25 of 83 strikes). Crew work alone
    -- does not share the attacker, as nobody else ends it then: counted shared for a warrior ten tiles off and out of
    -- sight, a poet under blows neither struck back nor got away (offline stage supply seed 18, 818-823s).
    local shared  = backed(snap, query)
    local window  = shared and combat.cast(snap) or fight.time
    local ignored = combat.lethal(snap, found.mob, window, snap.self.hp)
    local ended   = shared and math.min(1, window / math.max(fight.time, 1e-9)) or 1
    -- With a crew, a strike back is worth only its own share of the kill, net of its mana, and the mana of a poet is
    -- also the warrior's next heal: priced by its refill alone (6.6 a point) 화염주 beat every small heal, priced by
    -- the 건곤대나이 it fuels (about 45) it costs more than its share (online group, 16:08, 39 weighings). The fuel is
    -- priced on the mob I fight while the warrior rests too: priced by refill whenever it ate, 화염주 came out at
    -- 1455 over heals of 122-2293 (online group, 05:57, 58 of 83 strikes).
    local gain    = found.choice.value
    if shared then
        local price = math.max(value.spend(snap, query, found.mob), aid.fueling(snap, query, found.mob))
        gain        = math.min(gain, battle.marginal(snap, found) - (found.choice.mp or 0) * price)
        return gain + ignored * ended * danger.loss(snap, query, worth)
    end
    return math.max(0, gain + ignored * ended * danger.loss(snap, query, worth))
end

function M.attacked(snap, query)
    return snap.self.ghost == false and attacker(snap, query) ~= nil
end

function M.holds_ground(snap, query)
    local found = attacker(snap, query)
    return passing(snap, query, found.mob, combat.outcome(snap, query, found.mob)) == false
end

-- Striking back, the mobs on the crewmate the leader sent me to save are mine to strike too, weighed with the rescue
-- (battle.target): picking only from the mobs on me, a warrior beside its poet fought its own three skeletons while
-- three more beat the poet into a flight for 15s (offline stage group seed 1, 173-188s, run 08:29; user 11-11).
function M.strike_back(snap, query)
    return battle.propose(battle.target(snap, query, function(mob)
        return (battle.attacking(snap, mob) and battle.struck(snap, mob)) or party.rescue_share(snap, query, mob) > 0
    end))
end

return M
