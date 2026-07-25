-- spell: 성황령
-- 성황령 캐스팅

return {
    ON_CAST = function(me, spell, message)
        local map = me:map()
        if map == nil then
            return
        end

        if me:state() ~= STATE.GHOST then
            me:message('인간은 쓸 수 없음이니...')
            return
        end

        local root = map:model():root()
        local revive = root:revive();
        if revive == nil then
            return
        end

        local input = nil
        if message == '좌' then
            input = CARDINAL_DIRECTION.WEST
        elseif message == '우' then
            input = CARDINAL_DIRECTION.EAST
        else
            return
        end

        if revive[input] == nil then
            return
        end

        me:map(revive[input]:name())
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
