function on_cast(me, spell, message)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE_WORLD, BROADCAST_TYPE_GLOBAL)
end