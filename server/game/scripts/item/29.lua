-- item: 얼음칼

local spell = require('lib.spell')

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    on_attack = function(me, item)
        math.randomseed(seed())
        if math.random() > 0.3 then
            return false
        end

        local front = me:front(OBJECT_TYPE.LIFE)
        if front == nil then
            return
        end

        if front:isbuff(table.unpack(spell.relative_buff_name('마비'))) then
            return
        end

        front:sound(70)
        front:effect(1)
        front:buff('마비', 20, me)
    end
}
