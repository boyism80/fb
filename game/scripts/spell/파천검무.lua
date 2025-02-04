function on_cast(me, spell)
    local function failed(me)
        me:message(string.format('%s 실패', spell:name()))
    end

    local sound = 501
    local effect = 167
    local hp = me:hp()//2
    local mp = 180
    local damage = me:hp()

    local map = me:map()
    if map == nil then
        return failed(me)
    end

    if not map:movable(me, 5) then
        return failed(me)
    end

    local nears = me:nears(0xFF & ~OBJECT_TYPE_ITEM)
    local x, y = me:position()
    local direction = me:direction()
    local targets = {}
    for i = 1, 4 do
        local obj = front_obj(x, y, direction, i, nears, OBJECT_TYPE_LIFE)
        if obj ~= nil then
            table.insert(targets, obj)
        end
    end

    if #targets > 0 then
        me:position(me:front_position(5))
        attack_cast(me, targets, spell, hp, mp, damage, '파천검무', sound, effect)
        return
    else
        return failed(me)
    end
end