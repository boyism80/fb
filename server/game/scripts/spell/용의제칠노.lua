-- 용의제칠노: debuff (curse), AC 63, 12s, effect 53, sound 40.
function ON_CAST_54(me, you, spell)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 12
    if debuff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_54(me, spell)
    me:buff_phydef(me:buff_phydef() + 63)
end

function ON_UNBUFF_54(me, spell)
    me:buff_phydef(me:buff_phydef() - 63)
end
