-- spell: 중독
-- 중독 캐스팅
local spell = require('lib.spell')

-- 중독 버프 효과
-- 중독 버프 해제 효과
-- 중독 지속 효과

return {
    on_cast = function(me, you, sp)
        local mp = 30
        local sound = 68
        local effect = 14
        local buff_time = 30
        if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect, resist = RESIST.POISON}) then
            you:buff(sp, buff_time, me)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    on_concast = function(me, caster, buff)
        local damage = math.min((me:maxhp() * 5) // 100, 14994)
        if damage > me:hp() - 100 then
            damage = math.max(0, me:hp() - 100)
        end
        caster:damage_to(me, damage, { critical = false, rate = caster:skill_damage_rate() / 1000.0, physical = false })
    end
}
