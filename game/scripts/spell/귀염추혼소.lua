function on_cast(me, spell)
    local mp = 600
    local sound = 79
    local effect = 39

    for _, obj in pairs(me:nears(OBJECT_TYPE_LIFE)) do
        if not obj:isbuff(table.unpack(relative_buff_name(spell:model():name()))) then
            obj:effect(effect)
            obj:buff(spell, 425, me)
            if me ~= obj and obj:is(OBJECT_TYPE_CHARACTER) then
                obj:message(string.format('%s님이 %s 걸었습니다.', me:name(), name_with(spell:model():name())))
            end
        end
    end
    me:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
end

function on_buff(me, spell)
    me:buff_phydef(me:buff_phydef() + 50)
end

function on_unbuff(me, spell)
    me:buff_phydef(me:buff_phydef() - 50)
end