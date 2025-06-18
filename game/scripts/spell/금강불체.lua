-- 금강불체 캐스팅
function ON_CAST_4011(me, spell)
    local mp = 600
    local effect = 50
    local sound = 84
    local buff_time = 12

    local error = me:assert(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        return me:message(error)
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    local success = math.random() < 0.3
    if not debug() and not success then
        return me:message('실패')
    end

    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:buff(spell, buff_time)
    end
end

-- 금강불체 버프 효과
function ON_BUFF_4011(me, spell)
    me:invincible(true)
end

-- 금강불체 버프 해제 효과
function ON_UNBUFF_4011(me, spell)
    me:invincible(false)

end