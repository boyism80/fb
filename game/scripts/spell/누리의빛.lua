function on_cast(me, spell)
    local you = me:front()
    if you == nil then
        return
    end

    local mp = 30
    local effect = nil
    local sound = 19
    if not spell_cast(me, you, spell, mp, sound, effect) then
        return
    end

    if you:is(OBJECT_TYPE_CHARACTER) then
        local title = you:title()
        if title ~= '' then
            title = string.format('%s\n', title)
        end

        local message = string.format('%s %s 레벨 %d\n%s힘 : %d 민첩성 : %d 지능 : %d', you:class(), you:name(), you:level(), title, you:str(), you:dex(), you:int())
        me:message(message, MESSAGE_TYPE_POPUP)
    else
        me:message(you:name(), MESSAGE_TYPE_POPUP)
    end
end