-- spell: 흡성대법
-- 흡성대법 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local damage = 3000
        local mp = 60
        local sound = 38
        local effect = 44

        if not you:is(OBJECT_TYPE.MOB) then
            return me:message('걸리지 않습니다.')
        end

        if you:maxhp() > damage then
            return me:message('걸리지 않습니다.')
        end

        local before = you:hp()
        local cast, hit = spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
        if cast and hit then
            local absorbed = before - (you:hp() or 0)
            if absorbed > 0 then
                me:heal(absorbed)
            end
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
