-- 사자후도사 캐스팅
function ON_CAST_12(me, spell, message)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.SHOUT, BROADCAST_TYPE.WORLD)
    me:message('사자후도사를 외웠습니다.')
end