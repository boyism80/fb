-- 용의제오주: debuff (curse), AC 50, 6s, effect 53, sound 40.
local spell = require('lib.spell')

function ON_CAST_43(me, you, sp)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 6
    if spell.debuff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_43(me, sp)
    me:buff_phydef(me:buff_phydef() + 50)
end

function ON_UNBUFF_43(me, sp)
    me:buff_phydef(me:buff_phydef() - 50)
end
