local data     = require("brain.data.combat")
local items    = require("brain.data.items")
local magic    = require("brain.data.spells")
local bag      = require("brain.policies.bag")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local ground   = require("brain.policies.ground")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local score    = require("brain.policies.score")
local spacing  = require("brain.policies.spacing")
local survival = require("brain.policies.survival")
local value    = require("brain.policies.value")
local query    = require("brain.query")
local snapshot = require("brain.snapshot")
local trace    = require("brain.trace")

local M = {}

local function copy(source, changes)
    local result = {}
    for key, entry in pairs(source) do
        result[key] = entry
    end
    for key, entry in pairs(changes) do
        result[key] = entry
    end
    return result
end

local function slots(snap)
    local found = {}
    for _, learned in ipairs(snap.spells or {}) do
        found[learned.name] = learned.slot
    end
    return found
end

local function restore(effect, maximum)
    if effect == nil then
        return 0
    elseif effect.full then
        return maximum
    end
    return (effect.ratio or 0) * maximum + math.max(0, effect.amount or 0)
end

local function occupant(snap, x, y, except)
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.x == x and other.y == y and other.oid ~= except then
            return other
        end
    end
    return nil
end

local function around(snap, except)
    local found = {}
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.x ~= nil and other.oid ~= except and
            math.abs(other.x - snap.world.x) + math.abs(other.y - snap.world.y) == 1 then
            table.insert(found, other)
        end
    end
    return found
end

-- A swing splashes the tiles at my sides and my back as I face the mob it hits, each blow 8 times under 투명, as the
-- server's swing does (interaction on_attack, lib.interaction damage); only a swing from beside the mob has those
-- tiles known. Counted round the mob, the splash was credited on tiles the server never hits.
local function splashed(snap, mob, damage)
    local found = {}
    if mob.x == nil or damage <= 0 or math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) ~= 1 then
        return found
    end
    local x, y   = snap.world.x, snap.world.y
    local dx, dy = mob.x - x, mob.y - y
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    for name, splash in pairs(magic.splashes or {}) do
        if active[name] then
            local tiles = splash.sides == 1 and { { x - dx, y - dy } } or { { x + dy, y + dx }, { x - dy, y - dx } }
            for _, tile in ipairs(tiles) do
                local other = occupant(snap, tile[1], tile[2], mob.oid)
                if other ~= nil then
                    table.insert(found, { mob = other, damage = damage * splash.rate })
                end
            end
        end
    end
    return found
end

local function aside_gain(snap, extra)
    local total = 0
    for _, hit in ipairs(extra or {}) do
        local remaining = (hit.mob.hp or 0) * (hit.mob.ratio or 1)
        if remaining > 0 then
            total = total + combat.gain(hit.mob, snap) * math.min(1, hit.damage / remaining)
        end
    end
    return total
end

local RECENT = 2

local function beside(snap, mob)
    return mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 and combat.hits(mob)
end

-- A mob beside me attacks me when it attacks by itself, or when it only hits back and the fight is with it:
-- my target, already wounded, or the only one beside me that could have dealt the blow I just took.
-- Counting every retaliating neighbour made a 43 hp commoner flee from a stag and a squirrel that stood still
-- beside it (job gather_offering, online 03:16:55); counting only my target left a commoner hit 100ms ago by
-- the one squirrel beside it walking off instead of hitting back (state pure 6).
function M.attacking(snap, mob)
    if beside(snap, mob) == false then
        return false
    end
    local fight = snap.record.memory.battle
    if mob.attack ~= "RETALIATE" or (fight ~= nil and fight.oid == mob.oid) or (mob.ratio or 1) < 1 then
        return true
    end
    if M.struck(snap, mob) == false then
        return false
    end
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.oid ~= mob.oid and other.attack ~= "RETALIATE" and beside(snap, other) then
            return false
        end
    end
    return true
end

-- Struck means struck by a mob beside me within its own attack interval, not struck at any time before:
-- one blow two minutes earlier kept a full hp commoner "under attack" for good (job gather_offering).
function M.struck(snap, mob)
    return snap.self.struck ~= nil and snap.self.struck <= (mob.speed or 0) * RECENT
end

function M.pressed(snap)
    for _, mob in ipairs(snap.world.mobs or {}) do
        if M.attacking(snap, mob) and M.struck(snap, mob) then
            return true
        end
    end
    return false
end

local bursts = setmetatable({}, { __mode = "k" })

function M.burst(snap)
    local known = bursts[snap]
    if known ~= nil then
        return known
    end
    local attackers = {}
    for _, other in ipairs(snap.world.mobs or {}) do
        if M.attacking(snap, other) then
            table.insert(attackers, { mob = other, count = 1 })
        end
    end
    known = #attackers > 0 and combat.imminent(snap, attackers) or 0
    bursts[snap] = known
    return known
end

local reaches = setmetatable({}, { __mode = "k" })

function M.reachable(snap)
    if reaches[snap] ~= nil then
        return reaches[snap]
    end
    local worst = 0
    for _, other in ipairs(snap.world.mobs or {}) do
        if combat.hits(other) then
            worst = math.max(worst, combat.spike(snap, { { mob = other, count = 1 } }))
        end
    end
    reaches[snap] = worst
    return worst
end

local function bears(snap, facts, mob, choice)
    local mobs = {}
    for _, seen in ipairs(snap.world.mobs or {}) do
        if seen.oid ~= mob.oid then
            table.insert(mobs, seen)
        elseif (seen.hp or 0) * (seen.ratio or 1) > choice.damage then
            table.insert(mobs,
                copy(seen, { ratio = ((seen.hp or 0) * (seen.ratio or 1) - choice.damage) / math.max(seen.hp or 1, 1) }))
        end
    end
    local after = copy(snap, {
        self = copy(snap.self, { hp = snap.self.hp - choice.hp, mp = math.max(0, (snap.self.mp or 0) - choice.mp) }),
        world = copy(snap.world, { mobs = mobs }),
    })
    return danger.fatal(after, facts) == nil
end

function M.closing(snap)
    local window    = 2 * combat.cast(snap)
    local attackers = {}
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.x ~= nil and combat.hits(other) and (combat.seeks(other) or M.attacking(snap, other)) then
            local gap = math.abs(other.x - snap.world.x) + math.abs(other.y - snap.world.y) - 1
            if gap * (other.speed or math.huge) <= window then
                table.insert(attackers, { mob = other, count = 1 })
            end
        end
    end
    return #attackers > 0 and combat.imminent(snap, attackers) or 0
end

-- How long the hp a strike spends stays off me. A crewmate that heals the others heals one of us a cast, the hurt by
-- turns, so the hp is back after as many of its rounds as heals it takes, while its mana pays for them; else my own
-- doses or heal bring it back a cast at a time, and with neither it stays off for the rest of the fight. Counted as
-- the crew's whole heal on me, the hp was back within a cast and a warrior at a third of its hp struck again (online
-- trio 113100); counted for a poet out of mana, a warrior under blows struck down to 471 (fighter_awaits_dry_healer).
local function restored(snap, facts, mob, cost)
    local crew = escort.present(snap, facts)
    local rate = 0
    local most = 0
    local hurt = 1
    for _, member in ipairs(crew ~= nil and crew.members or {}) do
        if score.healer(member, facts) then
            rate = rate + 1 / combat.cast(member)
            most = math.max(most, score.capability(member, facts).blow.heal)
        end
        if member.self.hp < member.self.max_hp then
            hurt = hurt + 1
        end
    end
    if rate > 0 and most > 0 and crew.stock >= cost then
        return math.ceil(cost / most) * hurt / rate
    end
    local own  = recovery.heal(snap)
    local dose = 0
    for _, entry in ipairs(recovery.doses(snap, "hp")) do
        dose = math.max(dose, entry.amount)
    end
    local each = math.max(dose, own ~= nil and own.mp <= (snap.self.mp or 0) and own.hp or 0)
    if each > 0 then
        return math.ceil(cost / each) * combat.cast(snap)
    end
    return combat.outcome(snap, facts, mob).time
end

-- A strike that spends my hp is cast when the share of the mob's life it takes is more than the share of my own life it
-- puts at stake (user 11-14: both scored, no line between safe and not). At stake is what the mobs on me deal before the
-- hp is back (restored), against the hp the strike leaves me, net of what it is against the hp I have: from a third of
-- its hp the strike deals a third of the damage and leaves the blows a third of the room. Weighed by the chance to die
-- alone, that chance was nil beside one skeleton and a warrior struck again from a third of its hp 22 times, left at
-- 700-780 each time, and once fled at 522 with its poet beset five tiles off (online trio 123340). The mob's share is of
-- its whole life: of what its crew had left of it, a blow from a third of the hp came out a big share and the warrior
-- struck again from there 68 times (online trio 130022; user 11-14).
local function pays(snap, facts, mob, choice)
    if choice.hp <= 0 then
        return true
    end
    local wait  = combat.cast(snap) + restored(snap, facts, mob, choice.hp)
    local blows = 0
    local crit  = 1 + data.critical.chance * (data.critical.factor - 1)
    for _, other in ipairs(snap.world.mobs or {}) do
        if other.x ~= nil and combat.hits(other) and (other.speed or 0) > 0 and
            (other.oid == mob.oid or M.attacking(snap, other)) then
            local least, most = combat.spread(snap, other)
            blows             = blows + (least + most) / 2 * crit / other.speed
        end
    end
    local stake = 0
    if blows > 0 and wait == math.huge then
        stake = math.huge
    elseif blows > 0 then
        local taken = blows * wait
        stake       = taken / math.max(snap.self.hp - choice.hp, 1) - taken / math.max(snap.self.hp, 1)
    end
    local left = (mob.hp or 0) * (mob.ratio or 1)
    local take = (mob.hp or 0) > 0 and math.min(choice.damage, left) / mob.hp or 0
    return take > stake
end

local function gated(snap, facts, mob)
    if facts.warp == nil then
        return false
    end
    for _, side in ipairs({ { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } }) do
        if facts.warp(snap.world.map, mob.x + side[1], mob.y + side[2]) then
            return true
        end
    end
    return false
end

-- The ambush lands behind the mob or beside it (the server tries those tiles in turn) and only asks
-- whether the tile is free, not whether I can walk on from there. A rogue that landed in a sealed
-- two-tile pocket reached no ground any more: the group plan vanished, the poet left and the rogue stood
-- there 220s (profile 50+65 form, 682-685s). A tile I cannot walk back from is taken only when it still
-- reaches every map I reach now, so an ambush out of such a pocket stays open.
local function strands(snap, facts, mob)
    if facts.steps == nil or facts.reach == nil then
        return false
    end
    local dx, dy = mob.x - snap.world.x, mob.y - snap.world.y
    local here   = nil
    for _, tile in ipairs({ { mob.x + dx, mob.y + dy }, { mob.x + dy, mob.y + dx }, { mob.x - dy, mob.y - dx } }) do
        local back = tile[1] >= 0 and tile[2] >= 0 and
            facts.steps(snap.world.map, tile[1], tile[2], { { x = snap.world.x, y = snap.world.y, range = 0 } }) or nil
        if back ~= nil and back[1] == false then
            here        = here or facts.reach({}, {}, {
                map = snap.world.map,
                x = snap.world.x,
                y = snap.world.y,
                level = snap.self.level
            })
            local there = facts.reach({}, {}, { map = snap.world.map, x = tile[1], y = tile[2], level = snap.self.level })
            if here == nil or there == nil then
                return true
            end
            for map in pairs(here) do
                if there[map] == nil then
                    return true
                end
            end
        end
    end
    return false
end

-- The server lands the ambush behind the mob, else beside it, on a tile that is floor and free, and casts
-- nothing at all when none is. Its cost is 0 mp, so the cast still reads as done and the rogue stays cloaked:
-- among three stags it cast 2050 ambushes for 15 kills in 736s and never swung (RO seed 2, map 63).
-- Landed behind, the rogue faces the way the mob faces and the server doubles a blow on its back
-- (life::calculate_damage); landed beside, it faces the mob's flank and the blow is a plain one.
local function lands(snap, facts, mob)
    local dx, dy = mob.x - snap.world.x, mob.y - snap.world.y
    for index, tile in ipairs({ { mob.x + dx, mob.y + dy }, { mob.x + dy, mob.y + dx }, { mob.x - dy, mob.y - dx } }) do
        local free = facts.floor == nil or facts.floor(snap.world.map, tile[1], tile[2])
        for _, other in ipairs(snap.world.mobs or {}) do
            free = free and (other.x ~= tile[1] or other.y ~= tile[2])
        end
        for _, profile in ipairs(snap.party or {}) do
            local member = escort.member(profile)
            free         = free and (member.world.map ~= snap.world.map or member.world.x ~= tile[1] or
                member.world.y ~= tile[2])
        end
        if free then
            return index == 1 and magic.back_attack or 1
        end
    end
    return nil
end

-- Sides of a tile in the server's direction order: top, right, bottom, left.
local SIDES = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }

-- The ambush sets me past the object on the tile in front of me, mob or character: the server lands me behind it,
-- else on its left, else on its right, on the first of them that is floor and free (spell 비영승보), and casts nothing
-- when none is. A crewmate standing in a one-tile way is so jumped instead of waited on: a poet in the neck of
-- 흑해골굴1 kept its rogue off the skeletons beyond it (online trio 095311; user 11-14). A landing on a warp would end
-- on another floor, so there is no jump then.
function M.vault(snap, facts, x, y)
    local dx, dy = x - snap.world.x, y - snap.world.y
    if math.abs(dx) + math.abs(dy) ~= 1 then
        return nil
    end
    local found = nil
    for name, slot in pairs(slots(snap)) do
        local spell = magic.spells[name]
        if found == nil and spell ~= nil and spell.role == "AMBUSH" and (spell.mp or 0) <= (snap.self.mp or 0) then
            found = { key = name, maneuver = { kind = MANEUVER.CAST, slot = slot, self = false, cost = spell.mp or 0 } }
        end
    end
    if found == nil then
        return nil
    end
    local taken = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil then
            taken[mob.x .. "," .. mob.y] = true
        end
    end
    for _, tile in ipairs(snap.world.occupied or {}) do
        taken[tile.x .. "," .. tile.y] = true
    end
    for _, profile in ipairs(snap.party or {}) do
        local member = escort.member(profile)
        if member.world.map == snap.world.map and member.world.x ~= nil then
            taken[member.world.x .. "," .. member.world.y] = true
        end
    end
    local facing = dx > 0 and 2 or dx < 0 and 4 or dy < 0 and 1 or 3
    for _, turn in ipairs({ 0, 3, 1 }) do
        local side   = SIDES[(facing - 1 + turn) % 4 + 1]
        local tx, ty = x + side[1], y + side[2]
        if tx >= 0 and ty >= 0 and taken[tx .. "," .. ty] == nil and
            (facts.floor == nil or facts.floor(snap.world.map, tx, ty)) then
            if facts.warp ~= nil and facts.warp(snap.world.map, tx, ty) then
                return nil
            end
            found.x, found.y = tx, ty
            return found
        end
    end
    return nil
end

-- A walk stopped at a character in front of me goes on past it by the ambush when the landing is nearer the place
-- the walk is for than I stand (vault). Returns whether I moved.
function M.vault_past(brain, bot, event, x, y)
    if event.type ~= "OCCUPIED" or event.detail ~= "CHARACTER" or event.x == nil or event.oid == nil then
        return false
    end
    local snap = snapshot.take(brain, bot)
    local over = M.vault(snap, query.bind(brain, bot, snap), event.x, event.y)
    local here = math.abs(x - snap.world.x) + math.abs(y - snap.world.y)
    if over == nil or math.abs(x - over.x) + math.abs(y - over.y) >= here then
        return false
    end
    local before = bot:position()
    local leapt  = brain:engage(bot, event.oid, over.maneuver)
    local after  = bot:position()
    log("debug", string.format("[brain:vault] bot=%s over %d to %d,%d %s", bot:name(), event.oid, over.x, over.y,
        leapt.type))
    return before[1] ~= after[1] or before[2] ~= after[2]
end

local function sighted(snap, mob)
    return mob.x == nil or
        (math.abs(mob.x - snap.world.x) <= data.sight.x and math.abs(mob.y - snap.world.y) <= data.sight.y)
end

local function candidates(snap, facts, mob, state, reachable)
    local target    = combat.exposed(mob, combat.debuffs(snap, copy(mob, { applied = state.applied })))
    local options   = combat.options(snap, target)
    local known     = slots(snap)
    local cast      = combat.cast(snap) + combat.cooldown(snap)
    local swing     = data.swing
    local found     = {}
    local spendable = recovery.spendable(snap)
    local free      = recovery.free(snap)
    local seen      = sighted(snap, mob)
    -- The swing and the cast keep apart cooldowns: what I cast or drink while my swing still cools takes from the fight
    -- only the part that outlasts the cooldown (overlap), and the blows that land meanwhile land all the same. Counted
    -- whole, a rogue beside its mob waited out its swing after 투명 238 times where 비영승보 would have struck in the
    -- gap (online 120559); taken off the blows as well, a poet among skeletons weighed every action safe, never fled
    -- and died charging (offline WA+PO 세작 seed 6, run 15:41).
    local beside    = mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1
    local cooling   = beside and combat.backswing(snap) or 0
    -- A mob I cannot walk to is fought only with what I aim at it from here: a drink or a buff opens a fight my blows
    -- go on with, and weighed so, a warrior that could not walk round its poet to a skeleton drank 동동주 at it 331
    -- times a minute (offline WA+PO 세작 seed 0, run 18:35).
    local add       = function(entry)
        local aimed = entry.aimed == true or (entry.maneuver.kind == MANEUVER.CAST and entry.maneuver.self ~= true)
        local limit = entry.kind == "HEAL" and spendable or free
        if reachable == false and (aimed == false or entry.moves == true) then
            return
        end
        if entry.kind ~= "SWING" then
            entry.overlap = math.min(entry.time, cooling)
        end
        if state.refused[entry.key] == nil and (entry.mp or 0) <= limit and (seen or aimed == false) then
            table.insert(found, entry)
        end
    end
    table.sort(options.offense, function(left, right)
        return left.damage > right.damage
    end)
    local adjacent = mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1
    local burst    = M.burst(snap)
    local opener   = combat.opener(snap, mob)
    -- A strike that spends all the mana there is needs none kept for it, and a spell short of mana drinks the least
    -- dose that makes it up and is cast next (user 11-12). Kept for the strike, all the mana was held back, and a rogue
    -- that knew 백열장 never cast it at the mobs on its poet while carrying 5000 doses (offline RO+PO 세작, run 00:27).
    local drains   = recovery.draining(snap)
    local reserve  = drains and 0 or math.max(recovery.spare(snap), opener ~= nil and opener.mp or 0)
    local doses    = drains and recovery.doses(snap, "mp") or {}
    local mp       = snap.self.mp or 0
    for _, choice in ipairs(options.offense) do
        local spell = magic.spells[choice.name]
        local area  = spell ~= nil and spell.role == "AREA" and spell.type == "NORMAL"
        local dose  = nil
        if choice.mp + reserve > mp then
            for _, entry in ipairs(doses) do
                dose = dose or
                    (mp + math.min(entry.amount, (snap.self.max_mp or 0) - mp) >= choice.mp + reserve and entry or nil)
            end
        end
        if choice.damage > 0 and known[choice.name] ~= nil and (choice.mp + reserve <= mp or dose ~= nil) and
            (choice.hp >= 0 or snap.self.hp + choice.hp > burst) and ((area or choice.front) == false or adjacent) then
            local extra = {}
            if area then
                for _, other in ipairs(around(snap, mob.oid)) do
                    table.insert(extra, { mob = other, damage = choice.damage })
                end
            end
            local drunk = dose ~= nil and math.min(dose.amount, (snap.self.max_mp or 0) - mp) or 0
            local model = dose ~= nil and facts.item(dose.name) or nil
            add({
                key = choice.name,
                kind = "CAST",
                time = dose ~= nil and 2 * cast or cast,
                damage = choice.damage,
                mp = choice.mp - drunk,
                hp = choice.hp,
                price = dose ~= nil and model ~= nil and model.price or nil,
                extra = extra,
                aimed = dose ~= nil or nil,
                maneuver = dose ~= nil and { kind = MANEUVER.USE, item = dose.name } or
                    { kind = MANEUVER.CAST, slot = known[choice.name], cost = choice.mp }
            })
        end
    end
    -- A strike short of mana is drunk for as a spell is, by the bot whose strike spends all its mana: offered only
    -- with the mana already there, a rogue among its doses started four fights in five under the 60 its 필살검무
    -- costs and swung instead (online 120559, 123007).
    for _, choice in ipairs(options.strike) do
        local dose = nil
        if choice.mp > mp then
            for _, entry in ipairs(doses) do
                dose = dose or (mp + math.min(entry.amount, (snap.self.max_mp or 0) - mp) >= choice.mp and entry or nil)
            end
        end
        local drunk = dose ~= nil and math.min(dose.amount, (snap.self.max_mp or 0) - mp) or 0
        if known[choice.name] ~= nil and (choice.mp <= mp or dose ~= nil) and adjacent and
            pays(snap, facts, mob, choice) and bears(snap, facts, mob, choice) then
            local spell = magic.spells[choice.name]
            local model = dose ~= nil and facts.item(dose.name) or nil
            add({
                key = choice.name,
                kind = "STRIKE",
                time = dose ~= nil and 2 * cast or cast,
                damage = choice.damage,
                mp = choice.mp - drunk,
                spend = spell ~= nil and (spell.damage_mp or 0) > 0 and (recovery.opening(snap) or choice.mp) or nil,
                hp = -choice.hp,
                price = dose ~= nil and model ~= nil and model.price or nil,
                opener = true,
                aimed = dose ~= nil or nil,
                maneuver = dose ~= nil and { kind = MANEUVER.USE, item = dose.name } or
                    { kind = MANEUVER.CAST, slot = known[choice.name], cost = choice.mp }
            })
        end
    end
    for name, slot in pairs(known) do
        local spell = magic.spells[name]
        if spell ~= nil and spell.role == "DEBUFF" and state.applied[name] == nil and
            type(spell.mp or 0) == "number" and (spell.mp or 0) <= (snap.self.mp or 0) then
            add({
                key = name,
                kind = "DEBUFF",
                time = cast,
                mp = spell.mp or 0,
                debuff = name,
                maneuver = { kind = MANEUVER.CAST, slot = slot, cost = spell.mp or 0 }
            })
        end
    end
    local missing = snap.self.max_hp - snap.self.hp
    local mending = missing > 0 and recovery.mending(snap, missing) or nil
    local crew    = escort.present(snap, facts)
    local helped  = escort.ready(crew, cast)
    for _, choice in ipairs(options.heal) do
        if mending ~= nil and known[choice.name] == mending.slot and choice.hp > helped then
            add({
                key = choice.name,
                kind = "HEAL",
                time = cast,
                mp = choice.mp,
                hp = choice.hp - helped,
                wasted = choice.mp * math.max(0, 1 - missing / math.min(choice.hp, snap.self.max_hp)),
                maneuver = { kind = MANEUVER.CAST, slot = known[choice.name], self = true }
            })
        end
    end
    local charge = recovery.spells(snap).charge
    for name, count in pairs(bag.usable(snap)) do
        local effect = count > 0 and items[name] or nil
        if effect ~= nil then
            local hp    = restore({ full = effect.hp_full, ratio = effect.hp_ratio, amount = effect.hp },
                snap.self.max_hp)
            local mp    = restore({ full = effect.mp_full, ratio = effect.mp_ratio, amount = effect.mp },
                snap.self.max_mp or 0)
            local model = facts.item(name)
            if hp <= 0 and mp > 0 and charge ~= nil and (snap.self.mp or 0) >= charge.mp then
                mp = 0
            end
            mp = math.min(mp, (snap.self.max_mp or 0) - (snap.self.mp or 0))
            hp = math.min(hp, snap.self.max_hp - snap.self.hp)
            if hp > 0 or mp > 0 then
                add({
                    key = name,
                    kind = "USE",
                    time = cast,
                    hp = hp,
                    mp = -mp,
                    price = model ~= nil and model.price or 0,
                    maneuver = { kind = MANEUVER.USE, item = name }
                })
            end
        end
    end
    local charging = combat.buffs(snap).charge
    if charge ~= nil and charging ~= nil and (snap.self.mp or 0) >= charge.mp and free < recovery.prime(snap) then
        local change = recovery.charged(snap) - snap.self.hp
        local refill = (snap.self.max_mp or 0) - (snap.self.mp or 0) + charge.mp
        add({
            key = "CHARGE",
            kind = "CHARGE",
            time = cast,
            mp = charge.mp - charging.chance * refill,
            hp = charging.chance * change,
            outcomes = {
                { share = charging.chance, mp = charge.mp - refill, hp = change },
                { share = 1 - charging.chance, mp = charge.mp, hp = 0 },
            },
            maneuver = { kind = MANEUVER.CAST, slot = charge.slot, self = true }
        })
    end
    local strike = combat.strike(snap, facts, target)
    local melee  = strike / swing * 1000
    for _, entry in ipairs(found) do
        if entry.kind == "STRIKE" then
            entry.melee = melee
        end
    end
    local cloak = 1
    for name, slot in pairs(known) do
        local factor = (magic.multipliers or {})[name]
        local spell  = magic.spells[name]
        if factor ~= nil and snap.self.cloaked == true then
            cloak = math.max(cloak, factor)
        elseif factor ~= nil and spell ~= nil and (spell.mp or 0) <= (snap.self.mp or 0) then
            add({
                key = name,
                kind = "CAST",
                time = cast,
                damage = strike * (factor - 1),
                extra = splashed(snap, mob, strike * (factor - 1)),
                mp = spell.mp or 0,
                maneuver = { kind = MANEUVER.CAST, slot = slot, self = true, cost = spell.mp or 0 }
            })
        end
    end
    -- An ambush is one more blow struck on the cast's cooldown, cloaked or not: offered only under the cloak, a rogue
    -- whose 투명 a swing had spent swung on alone while the cast stood idle (online 120559).
    for name, slot in pairs(known) do
        local spell   = magic.spells[name]
        local landing = spell ~= nil and spell.role == "AMBUSH" and (spell.mp or 0) <= (snap.self.mp or 0) and
            adjacent and gated(snap, facts, mob) == false and lands(snap, facts, mob) or nil
        if landing ~= nil and strands(snap, facts, mob) == false then
            add({
                key = name,
                kind = "CAST",
                time = cast,
                damage = strike * cloak * landing,
                mp = spell.mp or 0,
                moves = true,
                maneuver = { kind = MANEUVER.CAST, slot = slot, cost = spell.mp or 0 }
            })
        end
    end
    if reachable ~= false then
        local ready = adjacent == false or combat.backswing(snap) <= 0
        add({
            key = "SWING",
            kind = "SWING",
            time = cast,
            damage = ready and strike * cloak or 0,
            extra = ready and splashed(snap, mob, strike * cloak) or {},
            approach = true,
            maneuver = { kind = MANEUVER.SWING }
        })
    end
    return found
end

local function delay(snap, mob, entry, regen, taken)
    local spent = -math.min(entry.hp or 0, 0)
    local heal  = regen - taken * 1000 / entry.time
    local top   = snap.self.max_hp
    if spent <= 0 or heal <= 0 or snap.self.hp >= top then
        return 0
    end
    local cast = entry.time / 1000
    local rate = function(damage, debt)
        local mend = debt / heal
        return (damage + entry.melee * mend) / (cast + mend)
    end
    local now  = rate(entry.damage, spent)
    if now <= 0 then
        return 0
    end
    local full = rate(entry.damage + top - snap.self.hp, spent * top / snap.self.hp)
    local wait = (top - snap.self.hp) / heal
    local left = (mob.hp or 0) * (mob.ratio or 1)
    return math.max(0, (full - now) * left / now - (now - entry.melee) * wait) / now
end

local function estimate(snap, facts, mob, entry, worth, loss, mana)
    local hits   = combat.hits(mob) and mob.speed ~= nil and mob.speed > 0
    local taken  = (hits and entry.time / mob.speed * combat.blow(snap, mob) or 0) +
        survival.contact(snap, mob, entry.time).pressure * entry.time
    local now    = combat.lethal(snap, mob, entry.time, snap.self.hp)
    local left   = (mob.hp or 0) * (mob.ratio or 1) - (entry.damage or 0)
    local hp     = math.min(snap.self.max_hp, math.max(0, snap.self.hp - taken) + math.max(0, entry.hp or 0)) +
        math.min(0, entry.hp or 0)
    local mp     = math.min(snap.self.max_mp or 0, math.max(0, (snap.self.mp or 0) - (entry.mp or 0)))
    local rest   = { time = 0, p_die = 0 }
    local ending = hp
    local refill = 0
    if left > 0 and hp > 0 then
        local plenty = combat.buffs(snap).charge
        local ahead  = recovery.heal(snap)
        local fresh  = { hp = hp, mp = mp }
        local mender = escort.present(snap, facts)
        if entry.kind == "STRIKE" and mender ~= nil and mender.heal > 0 then
            fresh.hp = math.min(snap.self.max_hp, hp + math.min(-entry.hp, mender.stock))
        end
        if entry.kind == "HEAL" and plenty ~= nil and ahead ~= nil and mp >= plenty.mp and mp < ahead.mp + plenty.mp and
            hp > plenty.cost + 2 * M.burst(snap) then
            fresh  = {
                hp = hp - plenty.chance * plenty.cost,
                mp = mp - plenty.mp + plenty.chance * ((snap.self.max_mp or 0) - mp + plenty.mp)
            }
            refill = combat.cast(snap)
        end
        local after   = copy(snap, { self = copy(snap.self, fresh) })
        local applied = {}
        for name in pairs(entry.applied or {}) do
            applied[name] = true
        end
        if entry.debuff ~= nil then
            applied[entry.debuff] = true
        end
        local prey            = copy(mob, { ratio = left / math.max(mob.hp or 1, 1), applied = applied })
        local context, fought = escort.context(after, facts, prey, escort.present(snap, facts), nil,
            recovery.rates(after).hp)
        rest                  = survival.joined(after, facts, prey, context,
            fought or survival.fight(after, facts, prey, context))
        ending                = rest.ending
    elseif hp <= 0 then
        rest = { time = 0, p_die = 1 }
    end
    local p_die = 1 - (1 - now) * (1 - rest.p_die)
    local blows = M.burst(snap)
    local near  = 0
    if blows > 0 and left > 0 then
        local cover = (entry.hp or 0) < 0 and hp or hp + escort.ready(escort.present(snap, facts), combat.cast(snap))
        near        = math.max(0, math.min(1, (2 * blows - cover) / blows))
        p_die       = 1 - (1 - p_die) * (1 - near)
    end
    local mend = recovery.heal(snap)
    if refill == 0 and entry.kind ~= "CHARGE" and combat.buffs(snap).charge ~= nil and mend ~= nil and hp > 0 and
        mp < mend.mp + combat.buffs(snap).charge.mp then
        local exposed = 1 - math.min(1, M.burst(snap) / hp)
        p_die = 1 - (1 - p_die) * (mp < combat.buffs(snap).charge.mp and exposed * exposed or exposed)
    end
    local time     = (entry.time - (entry.overlap or 0) + refill + rest.time) / 1000
    local ready    = ground.need(snap, facts)
    local charging = combat.buffs(snap).charge
    if charging ~= nil then
        ready = math.max(ready, charging.cost + math.max(2 * M.burst(snap), M.closing(snap)))
    end
    local short = math.max(0, ready - math.max(ending, 0))
    local crew  = escort.present(snap, facts)
    local regen = recovery.rates(snap).hp + recovery.refilling(snap, "hp") + (crew ~= nil and crew.heal or 0)
    if short > 0 and regen > 0 then
        time = time + short / regen
    end
    -- Hp a blow spends comes back at the rate I am healed, my crew's heals with it, as a strike's does (delay): left
    -- free beside a healer, 금강퇴 filled the cast between every two swings once its time overlapped the swing's
    -- cooldown, 1962 to 4119 casts a run, and 홍주 went 1555 to 1835 (offline WA+PO 흉가, runs 11:55 and 17:32).
    if entry.melee ~= nil then
        time = time + delay(snap, mob, entry, regen, taken)
    elseif (entry.hp or 0) < 0 and regen > 0 then
        time = time - entry.hp / regen
    end
    if (entry.wasted or 0) > 0 then
        local refill = recovery.rates(snap).mp + recovery.refilling(snap, "mp")
        time         = refill > 0 and time + entry.wasted / refill or time
    end
    local bonus = aside_gain(snap, entry.extra)
    local risk  = (1 - p_die) * (combat.gain(mob, snap) + bonus) - (p_die + near) * loss
    local plan  = {
        time = time * 1000,
        p_die = p_die,
        ending = ending,
        rest = rest.time,
        extra = #(entry.extra or {}),
        drained = rest.drained or 0
    }
    return
        risk - time * worth.value - (entry.price or 0) * worth.exchange -
        ((entry.spend or entry.mp or 0) + (rest.drained or 0)) * mana,
        ending > 0 and (entry.kind == "CHARGE" or
            entry.kind ~= "CHARGE" and hp > M.burst(snap) and hp > combat.spike(snap, { { mob = mob, count = 1 } })),
        ending,
        plan
end

-- An action that ends one of several ways is worth what each way is worth by its chance, and it survives only when
-- every way it can end does: weighed at the hp of its average way, a 공력증강 that more often than not left 114 hp
-- came out at 470, and a mage at 1,044 hp beside a spider charged and died (offline watch mage60 seed 0, run 18:20).
local function gamble(snap, facts, mob, entry, worth, loss, mana)
    local value    = 0
    local ending   = 0
    local survives = true
    local plan     = { time = 0, p_die = 0, ending = 0, rest = 0, extra = 0, drained = 0 }
    for _, outcome in ipairs(entry.outcomes) do
        local found, safe, left, planned = estimate(snap, facts, mob, copy(entry, { hp = outcome.hp, mp = outcome.mp }),
            worth, loss, mana)
        value                            = value + outcome.share * found
        ending                           = ending + outcome.share * left
        survives                         = survives and (outcome.share <= 0 or safe)
        for key in pairs(plan) do
            plan[key] = plan[key] + outcome.share * (planned[key] or 0)
        end
    end
    return value, survives, ending, plan
end

local function lasting(snap, name, at)
    if combat.lasting(name, at, snap.now) == false then
        return false
    end
    local spell = magic.spells[name]
    return spell == nil or (spell.paralysis ~= true and spell.blind ~= true) or snap.self.struck == nil or
        snap.self.struck > snap.now - at
end

local function remembered(snap, mob)
    local state  = { opened = false, refused = {}, applied = combat.marks(snap, mob.oid) }
    local memory = snap.record.memory.battle
    if memory ~= nil and memory.oid == mob.oid then
        state.opened = memory.opened == true
        if memory.refused ~= nil then
            state.refused[memory.refused] = true
        end
        if memory.spot ~= nil and memory.spot.x == snap.world.x and memory.spot.y == snap.world.y then
            for name in pairs(memory.denied or {}) do
                state.refused[name] = true
            end
        end
        for name, at in pairs(memory.applied or {}) do
            if lasting(snap, name, at) then
                state.applied[name] = true
            end
        end
    end
    for _, profile in ipairs(snap.party or {}) do
        local shared = profile.battle
        if type(shared) == "table" and shared.oid == mob.oid and shared.map == snap.world.map then
            for name, at in pairs(shared.applied or {}) do
                if lasting(snap, name, at) then
                    state.applied[name] = true
                end
            end
        end
    end
    return state
end

local evaluate = nil
local endings  = setmetatable({}, { __mode = "k" })

-- The hp a fight with a mob ends at is kept apart for each reach it was weighed with: kept as one, a mob weighed out
-- of reach, where blows cannot end it, left its endless fight for the hp a rogue needed beside it (battle.need), and
-- the rogue walked on to the next floor below half its hp (offline solo seed 201, 562s, run 19:39).
local function ended(mob, reachable)
    return tostring(mob.oid or mob.name) .. "|" .. tostring(reachable ~= false)
end
local targets = {}
local SAMPLE  = 5

local chosen = setmetatable({}, { __mode = "k" })

local function kindred(snap, facts, mob, flag)
    if mob.x == nil or mob.id == nil then
        return nil
    end
    local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
    local memory   = snap.record.memory.battle
    if distance <= 1 or (memory ~= nil and memory.oid == mob.oid) then
        return nil
    end
    for _, profile in ipairs(snap.party or {}) do
        if type(profile.battle) == "table" and profile.battle.oid == mob.oid then
            return nil
        end
    end
    -- A mob a rescue names is weighed for itself: the draw a spell earns at it, or the hold my blows put on it, is
    -- that mob's own (drawing, holding). Shared with its kind, the draw passed to mobs on nobody or was lost to the
    -- mob on the crewmate with whichever of them was weighed first: with the first listed weighed, poets took 3.40M
    -- for 2.51M and were struck 9,125s for 5,743s (offline 25 teams, runs 19:59 and 19:16).
    if #(snap.party or {}) > 0 then
        local oid       = tostring(mob.oid)
        local _, rescue = party.rescued(snap, facts)
        local awaited   = party.awaited(snap, facts)
        local drawn     = rescue ~= nil and (rescue.shares or {})[oid] ~= nil
        local held      = awaited ~= nil and (awaited.shares or {})[oid] ~= nil
        if drawn or held then
            return nil
        end
    end
    for _, name in ipairs(snap.buffs or {}) do
        if (magic.splashes or {})[name] ~= nil then
            return nil
        end
    end
    return table.concat({ mob.id, mob.ratio or 1, distance, tostring(sighted(snap, mob)), tostring(flag) }, "|")
end

local listed = setmetatable({}, { __mode = "k" })

-- Mobs of one kind at one distance share one weighing, that of the first the snapshot lists. Shared from whichever
-- was asked about first, a fight's worth hung on the order of the questions: asking fewer of them and nothing else
-- moved the fights of 4 of 65 episodes (offline 25 teams and solos, runs 19:33 and 19:48).
local function first_listed(snap, facts, key, flag)
    local byflag = listed[snap]
    if byflag == nil then
        byflag       = {}
        listed[snap] = byflag
    end
    local bykey = byflag[flag]
    if bykey == nil then
        bykey = {}
        for _, mob in ipairs(snap.world.mobs or {}) do
            local own = kindred(snap, facts, mob, flag)
            if own ~= nil and bykey[own] == nil then
                bykey[own] = mob
            end
        end
        byflag[flag] = bykey
    end
    return bykey[key]
end

-- A fight is weighed apart only for a mob I cannot walk to: named reachable or left open, its candidates are the
-- same. Kept under two keys, a mob was weighed twice in one decision, once to ask whether I would walk to it
-- (field.walking) and once to pick it (target): 162,094 weighings for 26,735 decisions (offline 8 teams, 5 minutes
-- each, run 19:36).
function M.choose(snap, facts, mob, reachable)
    local bysnap = chosen[snap]
    if bysnap == nil then
        bysnap = {}
        chosen[snap] = bysnap
    end
    local byreach = bysnap[mob]
    if byreach == nil then
        byreach = {}
        bysnap[mob] = byreach
    end
    local flag = reachable ~= false
    if byreach[flag] == nil then
        local key  = kindred(snap, facts, mob, flag)
        local same = key ~= nil and bysnap[key] or nil
        if same == nil then
            local first = key ~= nil and first_listed(snap, facts, key, flag) or mob
            same        = {
                choice = evaluate(snap, facts, first, reachable) or false,
                ending = endings[snap] ~= nil and endings[snap][ended(first, reachable)] or nil
            }
            if key ~= nil then
                bysnap[key] = same
            end
        end
        endings[snap]                        = endings[snap] or {}
        endings[snap][ended(mob, reachable)] = same.ending
        byreach[flag]                        = same.choice
    end
    return byreach[flag] or nil
end

-- A spell from afar that would make me the one a mob has taken the most damage from draws it off the crewmate it
-- strikes, which is what the leader sent me to do (party.rescue): a mob turns on whoever hurt it most once that one
-- leads the mob's target by its margin from afar (data.aggro.ranged), and the draw is worth the rescue in the share of
-- the crewmate's blows that mob deals (party.rescue_share). Beside the mob any blow draws it as well, so only the spell
-- from afar earns the rescue; a poet that dealt the mob nothing is drawn off at once, a mage whose spells hurt it most
-- is not. Once I lead, the mob is mine already and no spell earns the rescue again: every further spell earned it, so
-- warriors cast 백열장 from afar 8,230 times against 35 before (offline run 13:43). Short of the lead, a spell earns it in
-- the share of the gap it closes: earned by the overtaking spell alone, the first of the casts a poet's own damage
-- called for was worth nothing, so it was never cast and the poet stayed struck beside its crew (offline 25 teams, run
-- 20:15).
local function drawable(snap, facts, mob)
    local victim, rescue = party.rescued(snap, facts)
    local share          = rescue ~= nil and (rescue.shares or {})[tostring(mob.oid)] or nil
    if victim == nil or share == nil or victim.world.map ~= snap.world.map or victim.world.x == nil or mob.x == nil or
        math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1 then
        return 0
    end
    return rescue.value * share
end

local function drawing(snap, facts, mob, damage)
    local whole = drawable(snap, facts, mob)
    if whole <= 0 then
        return 0
    end
    local key  = tostring(mob.oid)
    local mine = (M.threat(snap.record.memory.dealt, snap.world.map)[key] or {}).damage or 0
    local most = 0
    for _, member in ipairs(party.members(snap)) do
        local entry = (member.dealt or {})[key]
        most        = math.max(most, entry ~= nil and entry.damage or 0)
    end
    local lead = most * data.aggro.ranged
    if mine > lead then
        return 0
    end
    return whole * math.min(1, damage / math.max(lead - mine, 1e-9))
end

-- My blows on a mob a crewmate is sent to draw off me hold it on me: it turns on whoever hurt it most, so each blow is
-- more the crewmate must deal, its casts and their mana from afar or its swings beside the mob. Uncounted, a poet with
-- nothing to heal struck back at the mob on it while its striker was sent to draw it, in 29 of 42 such moments, and
-- stayed struck (offline 25 teams, run 02:05; user 11-12).
function M.holding(snap, facts, mob, damage)
    local rescue, helper = party.awaited(snap, facts)
    if rescue == nil or (damage or 0) <= 0 or (rescue.shares or {})[tostring(mob.oid)] == nil then
        return 0
    end
    local strong = score.capability(helper, facts)
    local worth  = value.baseline(snap, facts)
    if rescue.walks == false and strong.blow.ranged > 0 then
        return damage / strong.blow.ranged *
            (strong.blow.mana * value.held(helper, facts, mob) + combat.cast(helper) / 1000 * worth.value)
    end
    local swing = ((helper.pace or {}).swing or data.swing) / 1000
    return damage / math.max(strong.blow.adjacent, 1e-9) * swing * worth.value
end

-- The same order picks the plan when none survives: taking the first listed of equal values, a solo warrior beaten
-- by a crab cast 백열장 rather than the swing it tied with (offline solo seed 201, run 06:06).
local function ahead(entry, other)
    return other == nil or entry.value > other.value or (entry.value == other.value and
        ((entry.damage or 0) > (other.damage or 0) or ((entry.damage or 0) == (other.damage or 0) and entry.key < other.key)))
end

function evaluate(snap, facts, mob, reachable)
    local baseline = value.baseline(snap, facts)
    local loss     = party.death(snap, facts, snap, baseline)
    local mana     = value.spend(snap, facts, mob)
    local best     = nil
    local ending   = -math.huge
    local last     = nil
    local state    = remembered(snap, mob)
    local listed   = candidates(snap, facts, mob, state, reachable)
    local melee    = nil
    local swung    = nil
    for _, entry in ipairs(listed) do
        if entry.kind == "SWING" then
            melee = entry
            melee.applied = state.applied
            swung = table.pack(estimate(snap, facts, mob, melee, baseline, loss, mana))
            melee.value, melee.plan = swung[1], swung[4]
        end
    end
    local share   = combat.gain(mob, snap) / math.max(mob.hp or 1, 1)
    local blocked = snap.record.memory.blocked
    local hemmed  = melee ~= nil and type(blocked) == "table" and snap.now - blocked.at < melee.plan.time
    local plan    = party.plan(snap, facts)
    for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
        hemmed = hemmed and floor.map ~= snap.world.map
    end
    local safest = 1
    for _, entry in ipairs(listed) do
        entry.applied = state.applied
        local left
        if entry == melee then
            entry.value, entry.survives, left, entry.plan = table.unpack(swung, 1, 4)
        else
            -- The rest of a fight with a mob I cannot walk to is fought from where I stand too: fought on with blows,
            -- 백열장 at a mob out of a warrior's reach came out worth a whole melee kill and was cast at mobs on nobody
            -- 8,468 times in a run (offline, run 18:59).
            entry.value, entry.survives, left, entry.plan = (entry.outcomes ~= nil and gamble or estimate)(snap, facts,
                reachable == false and copy(mob, { walled = true }) or mob, entry, baseline, loss,
                entry.kind == "STRIKE" and value.refill(snap, facts, baseline) or
                entry.kind == "USE" and value.restock(snap, facts, mob, -(entry.mp or 0)) or mana)
        end
        -- A spell that hits harder than a swing is worth the swing and the extra damage, less what its mana
        -- would make in a strike that mana holds back: counted free, 백열장 beat every swing and took the mana a
        -- warrior's 건곤대나이 needed (online group, 12:28). Without such a strike it stays free, as before. One
        -- that hits no harder is worth the swing less the more of that and its refill: priced by the refill alone,
        -- which a full bar makes nothing, 백열장 tied the swing of a solo warrior whose 건곤대나이 needed the mana.
        -- Both count every mob they hit: counted on the mob aimed at alone, 극진화열참주'첨 among four skeletons was
        -- worth 133 less than 극진화열참주 and was never cast (online WA+MA 152615, 127 casts among four).
        if entry.kind == "CAST" and melee ~= nil and hemmed == false and (entry.mp or 0) > 0 then
            local direct = math.max(0, (entry.damage or 0) * share + aside_gain(snap, entry.extra) -
                (melee.damage or 0) * share - aside_gain(snap, melee.extra))
            local empty  = 1 - (snap.self.mp or 0) / math.max(snap.self.max_mp or 1, 1)
            local held   = value.held(snap, facts, mob)
            entry.value  = direct > 0 and melee.value + direct - entry.mp * held or
                math.min(entry.value, melee.value - entry.mp * math.max(held, empty * value.sip(snap, facts, mob)))
        elseif entry.kind == "USE" and melee ~= nil and (entry.hp or 0) <= 0 then
            local gain  = entry.survives and -(entry.mp or 0) * value.restock(snap, facts, mob, -(entry.mp or 0)) or 0
            entry.value = math.min(entry.value, melee.value + gain)
        end
        -- Only a spell aimed at the mob from here draws it: 투명 counts the blows it makes harder as its damage, and
        -- counted as a draw it was a rogue's best answer to the mobs on its poet three tiles off, which it never
        -- touched (offline RO+PO 세작, run 23:21).
        if entry.kind == "CAST" and (entry.damage or 0) > 0 and entry.maneuver.self ~= true and entry.moves ~= true then
            entry.value = entry.value + drawing(snap, facts, mob, entry.damage)
        end
        entry.value = entry.value - M.holding(snap, facts, mob, entry.damage)
        if ahead(entry, last) then
            last = entry
        end
        ending = math.max(ending, left)
        safest = math.min(safest, entry.plan.p_die)
        if entry.survives and ahead(entry, best) then
            best = entry
        end
    end
    endings[snap] = endings[snap] or {}
    endings[snap][ended(mob, reachable)] = ending
    best = best or last
    if best ~= nil then
        best.safest = safest
    end
    if best ~= nil and (data.trace or data.journal) then
        best.rivals = {}
        for _, entry in ipairs(listed) do
            table.insert(best.rivals, { key = entry.key, value = entry.value })
        end
    end
    return best
end

function M.need(snap, facts)
    local nearest = nil
    local closest = math.huge
    for _, mob in ipairs(snap.world.mobs) do
        if mob.x ~= nil and combat.seeks(mob) and combat.hits(mob) then
            local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
            if distance < closest then
                nearest = mob
                closest = distance
            end
        end
    end
    if nearest == nil then
        return 0
    end
    local known = endings[snap] ~= nil and endings[snap][ended(nearest, nil)] or nil
    if known == nil then
        M.choose(snap, facts, nearest)
        known = endings[snap][ended(nearest, nil)]
    end
    return snap.self.hp - known + 1
end

-- A mob I cannot beat beside the only way left is passed when the blows taken on the way cost less than
-- waiting for the way to open. One that keeps up with my steps or kills in one blow is never passed
-- (danger.deadly), nor one whose blows on the way could end me. With a crewmate on the map the wait lasts
-- its walk to me; alone it lasts until the mob moves, which it did not.
function M.braves(snap, facts, mob, steps)
    if mob == nil or steps <= 0 or danger.deadly(snap, facts, mob) then
        return false
    end
    local step    = snap.pace.step or data.walk
    local _, high = combat.spread(snap, mob)
    local hit     = math.ceil(steps * step / math.max(mob.speed or step, 1)) * high
    if hit >= snap.self.hp then
        return false
    end
    local gap = math.huge
    for _, member in ipairs(party.members(snap)) do
        if member.self.ghost == false and member.world.map == snap.world.map and member.world.x ~= nil then
            gap = math.min(gap, math.abs(member.world.x - snap.world.x) + math.abs(member.world.y - snap.world.y))
        end
    end
    if gap == math.huge then
        return true
    else
        local worth = value.baseline(snap, facts)
        return hit * value.vigor(snap, facts, worth, value.mana(snap, facts)) < gap * step / 1000 * worth.value
    end
end

function M.marginal(snap, found)
    local mob = found.mob
    return (found.choice.damage or 0) / math.max(mob.hp or 1, 1) * combat.gain(mob, snap)
end

function M.target(snap, facts, accept)
    local step  = snap.pace.step or data.walk
    local plain = value.baseline(snap, facts)
    local worth = plain.value
    local found = nil
    local best  = nil
    local near  = {}
    local goals = {}
    -- A mob my walk could not reach is still struck from where I stand; one a spell from here was refused at is not.
    -- Dropped whole for the walk, a warrior behind its poet in a corridor one tile wide never weighed the 백열장 at
    -- the skeleton on the poet, and the poet swung at it alone for 720s (offline WA+PO 세작 seed 2, 180-900s, run 08:34).
    for _, mob in ipairs(snap.world.mobs) do
        local barred = mob.x ~= nil and M.blocked(snap, mob) or nil
        if mob.x ~= nil and (barred == nil or barred.cast ~= true) and accept(mob) then
            local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
            table.insert(near, { mob = mob, distance = distance, walled = barred ~= nil })
            table.insert(goals, { x = mob.x, y = mob.y, range = 1 })
        end
    end
    local paths = #goals > 0 and facts.steps ~= nil and
        facts.steps(snap.world.map, snap.world.x, snap.world.y, goals, nil, true) or {}
    for index, entry in ipairs(near) do
        local steps     = paths[index]
        entry.reachable = steps ~= false and entry.walled == false
        entry.steps     = type(steps) == "number" and steps or math.max(0, entry.distance - 1)
    end
    table.sort(near, function(left, right)
        return left.steps < right.steps
    end)
    local tiles = { { x = snap.world.x, y = snap.world.y } }
    for _, entry in ipairs(near) do
        table.insert(tiles, { x = entry.mob.x, y = entry.mob.y })
    end
    local gaps = #near > 0 and spacing.costs(snap, facts, tiles) or nil
    -- A mob is weighed only while the most it can be worth beats the best so far, and a spell that draws it off a
    -- crewmate is worth its kill and the draw: bounded by the kill alone, the 백열장 at a skeleton on its poet, worth
    -- 2.59M with the draw, was never weighed against the 1.6M of a warrior's own 건곤대나이 (offline profile 63+65, run
    -- 23:21).
    for index, entry in ipairs(near) do
        local travel = entry.reachable and entry.steps * step / 1000 * worth or 0
        local guard  = party.rescue_share(snap, facts, entry.mob)
        local apart  = gaps ~= nil and gaps[index + 1] - gaps[1] or 0
        local most   = combat.gain(entry.mob, snap) + guard + drawable(snap, facts, entry.mob) - math.min(apart, 0)
        if best == nil or most > best then
            local choice = M.choose(snap, facts, entry.mob, entry.reachable)
            local net    = choice ~= nil and
                choice.value + guard - (choice.approach and travel + apart or 0) or nil
            if net ~= nil and (entry.reachable or net > 0) and (best == nil or net > best) then
                found = { mob = entry.mob, choice = choice, steps = entry.steps, distance = entry.distance }
                best  = net
            end
        end
    end
    if found ~= nil then
        found.forecast = M.forecast(snap, facts, found.mob, found.choice)
        local owner    = snap.self.name or ""
        local last     = targets[owner]
        if last == nil or last.oid ~= found.mob.oid then
            local count    = (last ~= nil and last.count or 0) + 1
            targets[owner] = { oid = found.mob.oid, count = count }
            if count % SAMPLE == 1 then
                found.forecast.snapshot = M.capture(snap)
            end
        end
    end
    return found
end

function M.capture(snap)
    local memory = snap.record.memory or {}
    return {
        self = snap.self,
        buffs = snap.buffs,
        spells = snap.spells,
        gear = snap.gear,
        bag = snap.bag,
        world = { map = snap.world.map, x = snap.world.x, y = snap.world.y, mobs = snap.world.mobs, items = snap.world.items },
        record = { failures = {}, memory = { battle = memory.battle, perils = memory.perils } },
        pace = snap.pace,
        now = snap.now,
        persona = snap.persona,
        group = snap.group,
        party = snap.party,
    }
end

function M.forecast(snap, facts, mob, choice)
    local crew            = escort.present(snap, facts)
    local context, fought = escort.context(snap, facts, mob, crew)
    local fight           = fought or survival.fight(snap, facts, mob, context)
    local model           = combat.outcome(snap, facts, mob, nil, context.aid)
    local finite          = function(number)
        return number ~= nil and number < math.huge and number > -math.huge and number or -1
    end
    return {
        key = choice.key,
        kind = choice.kind,
        value = finite(choice.value),
        step = choice.plan,
        crew = crew ~= nil and #crew.members or 0,
        aid = finite(context.aid or 0),
        hp = snap.self.hp,
        mp = snap.self.mp or 0,
        ratio = mob.ratio or 1,
        fight = {
            time = finite(fight.time),
            taken = finite(fight.taken),
            healed = finite(fight.healed),
            ending = finite(fight.ending),
            p_die = finite(fight.p_die),
            spent = finite(fight.spent or 0)
        },
        model = {
            time = finite(model.time),
            casts = finite(model.casts),
            swings = finite(model.swings),
            taken = finite(model.taken),
            healed = finite(model.healed),
            drained = finite(model.drained)
        },
    }
end

-- A walk to strike goes no farther than the walk the strike was weighed with: walked to wherever the mob went, a
-- warrior beside its poet struck at a mantis beside it, followed it through a gap and walked 40 steps round the wall
-- after it, out of its poet's sight (offline WA63+PO65 seed 1, 795-806s, run 19:45). Nor does it take a way round
-- the bodies in between that is longer than that walk by more than a walk's leeway (decide): weighed at 14 steps
-- to the scorpion on its poet seven tiles off, a rogue took the way round the mobs between them, and its 14 steps
-- ended 17 tiles from the poet, each fighting alone for 37s (offline profile 50+65 form seed 4, 235-276s, run 04:28).
function M.propose(found, extra)
    local maneuver = found.choice.maneuver
    if found.choice.approach == true and found.steps ~= nil then
        maneuver = copy(maneuver, { steps = math.max(1, found.steps), leeway = escort.reach() })
    end
    local drinking = maneuver.kind == MANEUVER.USE
    local action   = {
        kind = ACTION.ATTACK,
        oid = found.mob.oid,
        maneuver = maneuver,
        key = drinking and maneuver.item or found.choice.key,
        opener = found.choice.opener == true,
        moves = found.choice.moves == true,
        debuff = found.choice.debuff,
        mob = found.mob.name,
        mob_id = found.mob.id,
        damage = drinking == false and found.choice.damage or nil,
        forecast = found.forecast,
        rivals = found.choice.rivals,
        scale = found.choice.value,
    }
    for key, entry in pairs(extra or {}) do
        action[key] = entry
    end
    return action
end

local GOING = { STRUCK = true, DONE = true, REFUSED = true, APPROACHED = true }

-- The damage I dealt each mob of this floor, posted so a crewmate can tell whether a blow of its own would draw
-- that mob off whoever it strikes. A mob keeps what each attacker dealt it until the attacker leaves its floor or dies
-- (the server's threat table): forgotten after 10s instead, a warrior holding three skeletons off its poet drew them
-- again every 10-11s (online 053346, 51 백열장). Hits are counted at the damage the choice expected: a client sees a
-- mob's hp only as a share, crewmates' blows included.
function M.threat(dealt, map)
    local kept = {}
    for oid, entry in pairs(dealt or {}) do
        if entry.map == map then
            kept[oid] = entry
        end
    end
    return kept
end

function M.tally(dealt, oid, action, event, now, map)
    local kept = M.threat(dealt, map)
    if event == "KILLED" then
        kept[tostring(oid)] = nil
    elseif event == "DONE" and (action.damage or 0) > 0 then
        local entry         = kept[tostring(oid)] or { damage = 0 }
        kept[tostring(oid)] = { damage = entry.damage + action.damage, at = now, since = entry.since or now, map = map }
    end
    return kept
end

function M.going(kind)
    return GOING[kind] == true
end

function M.blocked(snap, mob)
    local entry = (snap.record.memory.unreachable or {})[tostring(mob.oid)]
    return entry ~= nil and entry.map == snap.world.map and entry.x == mob.x and entry.y == mob.y and
        (entry.at == nil or snap.now - entry.at < data.rerank) and entry or nil
end

local function standing(blocked, visible, map)
    local kept  = {}
    local count = 0
    for key, entry in pairs(blocked or {}) do
        local mob = visible[key]
        if mob ~= nil and entry.map == map and mob.x == entry.x and mob.y == entry.y then
            kept[key] = entry
            count     = count + 1
        end
    end
    return kept, count
end

function M.settle(memory, blocked, oid, map, action, event, now, visible, moved, spot)
    if action.moves and event.type == "DONE" and moved == false then
        event = { type = "REFUSED" }
    end
    if memory == nil or memory.oid ~= oid or memory.map ~= map then
        memory = { oid = oid, map = map, opened = false }
    end
    memory.opened = memory.opened or action.opener == true
    if action.debuff ~= nil and event.type == "DONE" then
        memory.applied                = memory.applied or {}
        memory.applied[action.debuff] = now
    end
    memory.refused = event.type == "REFUSED" and action.key or nil
    if event.type == "REFUSED" and action.key ~= nil and action.maneuver.self ~= true then
        local denied = {}
        for name in pairs(memory.denied or {}) do
            denied[name] = true
        end
        denied[action.key] = true
        memory.denied      = denied
        memory.spot        = spot
    end
    local kept, count = standing(blocked, visible, map)
    local seen        = visible[tostring(oid)]
    local aimed       = event.type == "REFUSED" and action.maneuver.kind == MANEUVER.CAST and
        action.maneuver.self ~= true and
        action.moves ~= true
    local missed      = aimed and seen ~= nil and math.abs(seen.x - spot.x) + math.abs(seen.y - spot.y) > 1
    if (event.type == "UNREACHABLE" or event.type == "OCCUPIED" or missed) and seen ~= nil then
        local mob           = seen
        kept[tostring(oid)] = { map = map, x = mob.x, y = mob.y, at = now, cast = missed }
        count               = count + 1
    end
    return GOING[event.type] and memory or nil, count > 0 and kept or nil
end

-- A walk to a mob that found no way there is the hunter's UNREACHABLE: left unknown, the walk was chosen again every
-- period, and a warrior and its rogue stood 8 minutes by the warp of 동부여물망동 the mob lay beyond (online 183732,
-- 18:40:23-18:48:56).
function M.unreached(blocked, oid, map, now, visible)
    local kept, count = standing(blocked, visible, map)
    local mob         = visible[tostring(oid)]
    if mob ~= nil and mob.x ~= nil then
        kept[tostring(oid)] = { map = map, x = mob.x, y = mob.y, at = now }
        count               = count + 1
    end
    return count > 0 and kept or nil
end

-- Every swing waits out the previous one here, where it happens: a decision never picks a swing inside the
-- cooldown, but the job and tutorial hunts and the clearing of a blocked path swing outside decisions, and a
-- commoner swung 11 times in 311ms at a squirrel (job gather_offering, online). The approach walks at the
-- walking pace of the data for the same reason it does everywhere else. The cooldown runs from the moment
-- the swing is sent, which engage records: timed from the reply instead, every swing also waited the
-- round trip (DEFEND held 1746s longer over the solo scenario), and an engage that only approached
-- held back the next swing as if it had swung.
function M.step(brain, bot, oid, action)
    local swing = action.maneuver.kind == MANEUVER.SWING
    local pause = swing and combat.backswing(snapshot.take(brain, bot)) or 0
    if pause > 0 and brain:active(bot) then
        brain:watch(bot, oid, pause // 1 | 0)
    end
    local seen   = brain:observe(bot)
    local before = bot:position()
    local struck = nil
    for _, mob in ipairs(brain:mobs(bot)) do
        struck = mob.oid == oid and mob or struck
    end
    local event  = brain:engage(bot, oid, setmetatable({ pace = data.walk }, { __index = action.maneuver }))
    local target = nil
    for _, mob in ipairs(brain:mobs(bot)) do
        target = mob.oid == oid and mob or target
    end
    -- A spell at a mob that took none of its damage was cast at no one: the server takes the mana and strikes only
    -- what stands in front, and the mob's hp bar, sent before the cast is answered, stays where it was. Taken as cast,
    -- a warrior whose skeleton had stepped off cast 건곤대나이 three times in 0.9s at full hp (online trio 113100). It is
    -- refused then, as a cast the server turned down is. Only a blow the bar would show is told so: the bar counts
    -- whole percents.
    if event.type == "DONE" and action.maneuver.kind == MANEUVER.CAST and action.maneuver.self ~= true and
        struck ~= nil and target ~= nil and (target.ratio or 1) >= (struck.ratio or 1) and
        (action.damage or 0) >= (target.hp or math.huge) / 100 then
        event = { type = "REFUSED" }
    end
    if target ~= nil and target.x ~= nil and M.vault_past(brain, bot, event, target.x, target.y) then
        event = brain:engage(bot, oid, setmetatable({ pace = data.walk }, { __index = action.maneuver }))
    end
    local after   = bot:position()
    local visible = {}
    for _, mob in ipairs(brain:mobs(bot)) do
        visible[tostring(mob.oid)] = mob
    end
    trace.engage(brain, bot, oid, action, event.type)
    local memory, blocked = M.settle(brain:recall(bot, "battle"), brain:recall(bot, "unreachable"), oid, seen.map,
        action, event, brain:observe(bot).now, visible, before[1] ~= after[1] or before[2] ~= after[2],
        { x = after[1], y = after[2] })
    if data.journal and event.type == "REFUSED" then
        local mob = visible[tostring(oid)]
        log("debug",
            string.format("[brain:refused] bot=%s key=%s at=%s,%s mob=%s %s,%s hp=%s mp=%s", bot:name(),
                tostring(action.key),
                tostring(after[1]), tostring(after[2]), tostring(mob ~= nil and mob.name), tostring(mob ~= nil and mob.x),
                tostring(mob ~= nil and mob.y), tostring(mob ~= nil and mob.hp), tostring(bot:mp())))
    end
    brain:remember(bot, "battle", memory)
    brain:remember(bot, "unreachable", blocked)
    brain:remember(bot, "dealt",
        M.tally(brain:recall(bot, "dealt"), oid, action, event.type, brain:observe(bot).now, seen.map))
    return event
end

return M
