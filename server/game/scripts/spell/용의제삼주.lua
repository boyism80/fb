-- 용의제삼주: debuff (curse), AC 37, 6s, effect 53, sound 40.
function ON_CAST_41(me, you, spell)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 6
    if debuff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_41(me, spell)
    me:buff_phydef(me:buff_phydef() + 37)
end

function ON_UNBUFF_41(me, spell)
    me:buff_phydef(me:buff_phydef() - 37)
end
