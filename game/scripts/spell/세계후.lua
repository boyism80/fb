-- 세계후 캐스팅
function ON_CAST_5006(me, spell, message)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE_WORLD, BROADCAST_TYPE_GLOBAL)
end