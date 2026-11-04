-- spell: 헬파이어
-- 헬파이어 캐스팅
local spell = require('lib.spell')

local WEAPON_DELAY_REDUCTION = {
    [269] = 1,
    [270] = 1,
    [271] = 2,
    [272] = 2,
    [273] = 3,
    [274] = 3,
    [275] = 4,
}

return {
    on_cast = function(me, you, sp)
        local delay = 7
        if me:is(OBJECT_TYPE.CHARACTER) then
            local weapon = me:weapon()
            if weapon ~= nil then
                delay = delay - (WEAPON_DELAY_REDUCTION[weapon:model():id()] or 0)
            end
        end

        local damage = math.floor(me:mp() * 1.5)
        local mp = me:mp()
        local sound = 73
        local effect = 8
        if spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect }) and me:is(OBJECT_TYPE.CHARACTER) then
            me:spell(sp):delay(delay)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
