-- 용의제팔노: debuff (curse), AC 65, 13s, effect 53, sound 40.
function ON_CAST_55(me, you, spell)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 13
    if debuff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_55(me, spell)
    me:buff_phydef(me:buff_phydef() + 65)
end

function ON_UNBUFF_55(me, spell)
    me:buff_phydef(me:buff_phydef() - 65)
end
