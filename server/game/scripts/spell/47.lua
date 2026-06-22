-- spell: 용의제구주
-- 용의제구주: debuff (curse), AC 80, 15s, effect 53, sound 40.
local spell = require('lib.spell')

function ON_CAST_47(me, you, sp)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 15
    if spell.debuff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_47(me, sp)
    me:buff_phydef(me:buff_phydef() + 80)
end

function ON_UNBUFF_47(me, sp)
    me:buff_phydef(me:buff_phydef() - 80)
end
