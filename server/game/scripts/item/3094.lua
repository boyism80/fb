-- item: 갈색시약
local spell = require('lib.spell')

local BUFF = '시약무장'
local MP = 30

return {
    on_activated = function(me, item)
        local name = item:model():name()
        if me:mp() < MP then
            me:message('마력이 부족합니다.')
            me:mkitem(name, 1)
            return
        end
        if me:isbuff(table.unpack(spell.relative_buff_name(BUFF))) then
            me:message('이미 걸려있습니다.')
            me:mkitem(name, 1)
            return
        end
        me:cast(BUFF)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
