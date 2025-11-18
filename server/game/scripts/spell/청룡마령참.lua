-- 청룡마령참 캐스팅
function ON_CAST_7013(me, you, spell)
    you:effect(0x89)
    you:sound(0x08)
    if you:is(OBJECT_TYPE.CHARACTER) then
        you:message(string.format('%s가 청룡마령참을 가합니다.', me:name()))
    end

    me:action(0x03)
end