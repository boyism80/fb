-- item: 얼음칼

local spell = require('lib.spell')

return {
    -- ON_ACTIVATED = function(me, item)
    -- end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    ON_ATTACK = function(me, item)
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
