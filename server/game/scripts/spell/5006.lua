-- spell: 세계후
-- 세계후 캐스팅

return {
    on_cast = function(me, spell, message)
        broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.GLOBAL)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
