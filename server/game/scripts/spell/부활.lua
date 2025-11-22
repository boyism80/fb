-- 부활 캐스팅
function ON_CAST_4014(me, you, spell)
    if not you:is(OBJECT_TYPE.LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end
    
    local error = nil
    if me == you then
        error = me:assert(STATE.RIDING)
    else
        error = me:assert(STATE.RIDING, STATE.GHOST)
    end
    if error ~= nil then
        return me:message(error)
    end
    if not spell_cast(me, you, spell, {mp=300, sound=8, effect=11, no_assert=true}) then
        return
    end

    if you:is(OBJECT_TYPE.CHARACTER) and you:state() == STATE.GHOST then
        you:state(STATE.NORMAL)
        you:hp(math.max(30, you:hp()))
    end
end