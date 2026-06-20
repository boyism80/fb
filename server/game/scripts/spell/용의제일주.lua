local spell = require('lib.spell')

function ON_CAST_39(me, you, sp)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 6
    if spell.debuff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_39(me, sp)
    me:buff_phydef(me:buff_phydef() + 31)
end

function ON_UNBUFF_39(me, sp)
    me:buff_phydef(me:buff_phydef() - 31)
end
