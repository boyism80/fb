-- 공력증강 캐스팅
function ON_CAST_4002(me, spell)
    local mp = 30
    local effect = 11
    local sound = 65

    local error = me:assert(STATE.GHOST, STATE.RIDING)
    if error ~= nil then
        return me:message(error)
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    local percent = 0.5 + ((me:int() / 255) * 0.5)
    local success = boolean_random(percent)
    if not success then
        return me:message('정신집중에 실패하였습니다.')
    end

    me:mp(me:maxmp(), false)
    me:hp(math.max(100, me:hp() - math.floor(me:maxmp() * 0.4)), false)
    me:update()
    me:effect(effect)
    me:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    return true
end