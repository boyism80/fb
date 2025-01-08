function on_cast(me, spell, message)
    local map = me:map()
    if map == nil then
        return
    end

    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return
    end

    local input = nil
    if message == '동' or message == '1' then
        input = CARDINAL_DIRECTION_EAST
    elseif message == '서' or message == '2' then
        input = CARDINAL_DIRECTION_WEST
    elseif message == '남' or message == '3' then
        input = CARDINAL_DIRECTION_SOUTH
    elseif message == '북' or message == '4' then
        input = CARDINAL_DIRECTION_NORTH
    else
        return
    end

    local root = map:model():root()
    local cardinal = root:cardinal(input)
    if cardinal == nil then
        return
    end

    local dest, left, top, right, bottom = table.unpack(cardinal)
    math.randomseed(seed())
    local x = math.random(left, right)
    local y = math.random(top, bottom)
    me:map(dest:name(), x, y)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
end