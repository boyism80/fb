-- spell: 사자후전사
-- 사자후전사 캐스팅

return {
    ON_CAST = function(me, spell, message)
        broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.SHOUT, BROADCAST_TYPE.WORLD)
        me:message('사자후전사를 외웠습니다.')
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
