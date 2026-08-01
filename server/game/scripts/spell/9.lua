-- spell: 사자후전사
-- 사자후전사 캐스팅

return {
    on_cast = function(me, spell, message)
        broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.SHOUT, BROADCAST_TYPE.WORLD)
        me:message('사자후전사를 외웠습니다.')
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
