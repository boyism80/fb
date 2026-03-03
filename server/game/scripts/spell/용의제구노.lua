-- 용의제구노: debuff (curse), AC 80, 15s, effect 53, sound 40.
function ON_CAST_56(me, you, spell)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 15
    if debuff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_56(me, spell)
    me:buff_phydef(me:buff_phydef() + 80)
end

function ON_UNBUFF_56(me, spell)
    me:buff_phydef(me:buff_phydef() - 80)
end
