function on_cast(me, spell, message)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE_SHOUT, BROADCAST_TYPE_WORLD)
    me:message('사자후도적을 외웠습니다.')
end