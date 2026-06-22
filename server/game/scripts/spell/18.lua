-- spell: 시약무장
local spell = require('lib.spell')

function ON_CAST_18(me, you, sp)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if spell.buff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_18(me, sp)
    me:buff_phydef(me:buff_phydef() - 10)
end

function ON_UNBUFF_18(me, sp)
    me:buff_phydef(me:buff_phydef() + 10)
end
