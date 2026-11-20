-- spell: 귀염추혼소
-- 귀염추혼소 캐스팅
local spell = require('lib.spell')

-- 귀염추혼소 버프 효과

-- 귀염추혼소 버프 해제 효과

return {
    on_cast = function(me, sp)
        local mp = 600
        local sound = 79
        local effect = 39
        local buff_time = 425
        local delay = 30

        local hits = spell.area_debuff_cast(me, sp, { mp = mp, sound = sound, effect = effect })
        for _, you in ipairs(hits) do
            you:buff(sp, buff_time, me)
        end
        if #hits > 0 and me:is(OBJECT_TYPE.CHARACTER) then
            me:spell(sp):delay(delay)
        end
    end,

    on_buff = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 50)
    end,

    on_unbuff = function(me, sp)
        me:buff_phydef(me:buff_phydef() - 50)
    end,

    -- on_concast = function(me, sp)
    -- end
}
