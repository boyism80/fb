function on_cast(me, you, spell)
    local mp = 500
    local sound = 85
    local effect = 10
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        return me:message(error)
    end

    if me:mp() < mp then
        return me:message('마력이 부족합니다.')
    end
    me:mp_down(mp)

    local success = math.random() < 0.5
    if not debug() and not success then
        return me:message('무력화 실패')
    end

    you:effect(effect)
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:model():name())))
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:model():name())))
    end
    for _, buff in pairs(you:buffs()) do
        you:unbuff(buff)
    end
end