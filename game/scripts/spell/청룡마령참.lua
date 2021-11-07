function on_cast(me, you, spell)
    if you == nil then
        return false
    end

    you:effect(0x89)
    you:sound(0x08)
    you:message(string.format('%s가 청룡마령참을 가합니다.', me:name()))

    me:action(0x03)
end