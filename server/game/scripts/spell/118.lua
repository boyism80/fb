-- spell: 오성금강퇴
-- 오성금강퇴: deal damage to the target in front; consumes caster HP.
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local front = me:front(OBJECT_TYPE.LIFE)
        if front == nil then
            me:message('대상이 없습니다.')
            return
        end
        local hp_cost = 3840
        local damage = 3840
        if me:hp() < hp_cost then
            me:message('체력이 부족합니다.')
            return
        end
        me:hp(me:hp() - hp_cost)
        spell.damage(me, front, sp, { damage = damage, mp = 0, sound = 8, effect = 32 })
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
