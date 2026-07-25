-- spell: 맵번호
-- 맵번호 캐스팅

return {
    ON_CAST = function(me, spell, name)
        local map = name2map(name)
        if map == nil then
            me:message('맵 정보 없음', MESSAGE_TYPE.POPUP)
        else
            me:message(map:id(), MESSAGE_TYPE.POPUP)
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
