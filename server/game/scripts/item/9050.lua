-- item: 의태시약

local spell = require('lib.spell')

return {
    on_activated = function(me, item)
        local name = item:model():name()
        local front = spell.front_exclude_item(me)
        if front == nil then
            me:message('앞에 대상이 없습니다.')
            me:mkitem(name, 1)
            return
        end
        if me:isbuff(table.unpack(spell.relative_buff_name('의태'))) then
            me:message('이미 걸려있습니다.')
            me:mkitem(name, 1)
            return
        end

        me:cast(front, '의태')
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
