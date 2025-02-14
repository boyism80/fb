function on_cast(me, spell)
    local mp = 0
    local sound = 109
    local effect = 116
    local buff_time = 45
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time, me)
    end
end

function on_buff(me, spell)

end

function on_unbuff(me, spell)

end

function on_concast(me, caster, buff)
    local x, y = me:position()
    local begin_x = x-3
    local begin_y = y-3
    local end_x = x+3
    local end_y = y+3
    local targets = {}
    local spell = name2spell('신령지익진')
    for _, obj in pairs(me:nears(OBJECT_TYPE_LIFE)) do
        local obj_x, obj_y = obj:position()
        if not obj:isbuff(spell) and obj_x >= begin_x and obj_x <= end_x and obj_y >= begin_y and obj_y <= end_y then
            table.insert(targets, obj)
        end
    end

    local count = 0
    for _, obj in pairs(targets) do
        count = count + 1
        obj:effect(117)
        obj:buff(spell, 2, me)
        if me ~= obj and obj:is(OBJECT_TYPE_CHARACTER) then
            obj:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:model():name())))
        end
    end

    if buff:time() % 2 == 0 then
        me:effect(116)
    end

    if count > 0 then
        me:sound(109)
    end
end