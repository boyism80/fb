-- 용의제일노: debuff (curse), AC 51, 6s, effect 53, sound 40.
function ON_CAST_48(me, you, spell)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 6
    if debuff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_48(me, spell)
    me:buff_phydef(me:buff_phydef() + 51)
end

function ON_UNBUFF_48(me, spell)
    me:buff_phydef(me:buff_phydef() - 51)
end
