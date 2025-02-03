function on_cast(me, you, spell)
    local mp = 30
    local effect = nil
    local sound = 19
    if not spell_cast(me, you, spell, mp, sound, effect) then
        return
    end

    if you:is(OBJECT_TYPE_CHARACTER) then
        local title = you:title()
        if title == nil then
            title = ''
        else
            title = string.format('%s\n', title)
        end

        local message = string.format('%s %s 레벨 %d\n%s힘 : %d 민첩성 : %d 지능 : %d', you:class(), you:name(), you:level(), title, you:str(), you:dex(), you:int())
        local item_names = {}
        for _, item in pairs(you:items()) do
            table.insert(item_names, item:name())
        end

        if #item_names > 0 then
            message = string.format('%s\n가진 물건 : %s', message, table.concat(item_names, ', '))
        end

        me:message(message, MESSAGE_TYPE_POPUP)
    elseif you:is(OBJECT_TYPE_MOB) then
        local message = you:name()
        local item_names = {}
        for _, item in pairs(you:model():drop()) do
            table.insert(item_names, item:name())
        end

        if #item_names > 0 then
            message = string.format('%s\n가진 물건 : %s', message, table.concat(item_names, ', '))
        end
        me:message(message, MESSAGE_TYPE_POPUP)
    else
        me:message(you:name(), MESSAGE_TYPE_POPUP)
    end
end