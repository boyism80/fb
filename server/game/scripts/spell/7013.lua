-- spell: 청룡마령참
-- 청룡마령참 캐스팅

return {
    ON_CAST = function(me, you, spell)
        you:effect(0x89)
        you:sound(0x08)
        if you:is(OBJECT_TYPE.CHARACTER) then
            you:message(string.format('%s가 청룡마령참을 가합니다.', me:name()))
        end

        me:action(0x03)
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
