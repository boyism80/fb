-- spell: 금고주인의저주
-- 금고주인의저주 버프 해제 효과

return {
    -- on_cast = function(me, you, sp)
    -- end,

    on_buff = function(me, sp)
        me:damage_derate(me:damage_derate() - 49)
    end,

    on_unbuff = function(me, sp)
        me:damage_derate(me:damage_derate() + 49)
    end,

    -- on_concast = function(me, sp)
    -- end
}
