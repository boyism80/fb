-- 용의제육주: debuff (curse), AC 60, 21s, effect 53, sound 40.
local spell = require('lib.spell')

function ON_CAST_44(me, you, sp)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 21
    if spell.debuff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_44(me, sp)
    me:buff_phydef(me:buff_phydef() + 60)
end

function ON_UNBUFF_44(me, sp)
    me:buff_phydef(me:buff_phydef() - 60)
end
