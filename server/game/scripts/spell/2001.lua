-- spell: 누리의빛
-- 누리의빛 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local you = me:front()
        if you == nil then
            return
        end

        local mp = 30
        local effect = nil
        local sound = 19
        if not spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
            return
        end

        if you:is(OBJECT_TYPE.CHARACTER) then
            local title = you:title()
            if title ~= '' then
                title = string.format('%s\n', title)
            end

            local message = string.format('%s %s 레벨 %d\n%s힘 : %d 민첩성 : %d 지능 : %d', you:class(), you:name(), you:level(), title, you:str(), you:dex(), you:int())
            me:message(message, MESSAGE_TYPE.POPUP)
        else
            me:message(you:name(), MESSAGE_TYPE.POPUP)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
