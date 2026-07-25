-- spell: 잠복근무
-- 잠복근무 캐스팅

return {
    on_cast = function(me, spell)
        me:super_hide(true)
        me:state(STATE.CLOACK)
        me:effect(20)
        me:sound(28)
        me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
