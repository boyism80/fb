-- 잠복근무 캐스팅
function ON_CAST_5003(me, spell)
    me:super_hide(true)
    me:state(STATE.HALF_CLOACK)
end