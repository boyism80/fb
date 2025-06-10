-- 부활 캐스팅
function ON_CAST_4014(me, you, spell)
    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end
    
    local error = nil
    if me == you then
        error = me:assert_state(STATE_RIDING)
    else
        error = me:assert_state(STATE_RIDING, STATE_GHOST)
    end
    if error ~= nil then
        return me:message(error)
    end
    if not spell_cast(me, you, spell, {mp=300, sound=8, effect=11, no_assert=true}) then
        return
    end

    if you:is(OBJECT_TYPE_CHARACTER) and you:state() == STATE_GHOST then
        you:state(STATE_NORMAL)
        you:hp(math.max(30, you:hp()))
    end
end