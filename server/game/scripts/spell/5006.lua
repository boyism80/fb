-- spell: 세계후
-- 세계후 캐스팅

return {
    ON_CAST = function(me, spell, message)
        broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.GLOBAL)
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
