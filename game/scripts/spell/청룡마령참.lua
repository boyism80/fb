function on_cast(me, you, spell)
    you:effect(0x89)
    you:sound(0x08)
    you:message(string.format('%s가 청룡마령참을 가합니다.', me:name()))

    me:action(0x03)
end