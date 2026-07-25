-- spell: 잠복근무
-- 잠복근무 캐스팅

return {
    ON_CAST = function(me, spell)
        me:super_hide(true)
        me:state(STATE.CLOACK)
        me:effect(20)
        me:sound(28)
        me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
