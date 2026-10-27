-- spell: 백호의희원
-- 백호의희원 캐스팅
local spell = require('lib.spell')

local WEAPON_DELAY_PERCENT = {
    ['용겸제삼봉'] = 10,
    ['용겸제사봉'] = 15,
    ['용겸제오봉'] = 20,
    ['용겸제육봉'] = 25,
    ['용겸제칠봉'] = 30,
    ['용겸제팔봉'] = 35,
    ['용겸제구봉'] = 40,
}

return {
    on_cast = function(me, you, sp)
        local hp = me:mp() * 2
        local mp = me:mp() // 2
        local sound = 95
        local effect = 64
        local delay = math.min(60, me:maxmp() // 10000)
        if me:is(OBJECT_TYPE.CHARACTER) then
            local weapon = me:weapon()
            if weapon ~= nil then
                local pct = WEAPON_DELAY_PERCENT[weapon:name()]
                if pct ~= nil then
                    delay = delay - delay * pct // 100
                end
            end
        end
        if spell.heal(me, you, sp, { hp = hp, mp = mp, sound = sound, effect = effect }) then
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
