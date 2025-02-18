function on_cast(me, spell, message)
    local map = me:map()
    if map == nil then
        return
    end

    if me:state() ~= STATE_GHOST then
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
        input = CARDINAL_DIRECTION_WEST
    elseif message == '우' then
        input = CARDINAL_DIRECTION_EAST
    else
        return
    end

    if revive[input] == nil then
        return
    end

    me:map(revive[input]:name())
end