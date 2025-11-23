-- 사자후도적 캐스팅
function ON_CAST_10(me, spell, message)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.SHOUT, BROADCAST_TYPE.WORLD)
    me:message('사자후도적을 외웠습니다.')
end