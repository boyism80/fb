-- 맵번호 캐스팅
function ON_CAST_5005(me, spell, name)
    local map = name2map(name)
    if map == nil then
        me:message('맵 정보 없음', MESSAGE_TYPE_POPUP)
    else
        me:message(map:id(), MESSAGE_TYPE_POPUP)
    end
end