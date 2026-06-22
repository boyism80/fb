-- spell: 잠복근무
-- 잠복근무 캐스팅
function ON_CAST_5003(me, spell)
    me:super_hide(true)
    me:state(STATE.CLOACK)
    me:effect(20)
    me:sound(28)
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
end
