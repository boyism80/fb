-- 용의제이노: debuff (curse), AC 55, 6s, effect 53, sound 40.
function ON_CAST_49(me, you, spell)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 6
    if debuff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_49(me, spell)
    me:buff_phydef(me:buff_phydef() + 55)
end

function ON_UNBUFF_49(me, spell)
    me:buff_phydef(me:buff_phydef() - 55)
end
