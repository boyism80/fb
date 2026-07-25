-- spell: 맵번호
-- 맵번호 캐스팅

return {
    on_cast = function(me, spell, name)
        local map = name2map(name)
        if map == nil then
            me:message('맵 정보 없음', MESSAGE_TYPE.POPUP)
        else
            me:message(map:id(), MESSAGE_TYPE.POPUP)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
