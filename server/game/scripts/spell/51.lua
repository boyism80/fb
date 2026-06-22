-- spell: 용의제사노
-- 용의제사노: debuff (curse), AC 63, 6s, effect 53, sound 40.
local spell = require('lib.spell')

function ON_CAST_51(me, you, sp)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 6
    if spell.debuff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_51(me, sp)
    me:buff_phydef(me:buff_phydef() + 63)
end

function ON_UNBUFF_51(me, sp)
    me:buff_phydef(me:buff_phydef() - 63)
end
