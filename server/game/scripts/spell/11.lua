-- spell: 사자후술사
-- 사자후술사 캐스팅
function ON_CAST_11(me, spell, message)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.SHOUT, BROADCAST_TYPE.WORLD)
    me:message('사자후술사를 외웠습니다.')
end
