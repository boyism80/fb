-- spell: 금강불체
-- 금강불체 캐스팅
local spell = require('lib.spell')

-- 금강불체 버프 효과

-- 금강불체 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local mp = 600
        local effect = 50
        local sound = 84
        local buff_time = 12

        local error = me:assert(STATE.GHOST, STATE.RIDING)
        if error ~= nil then
            return me:message(error)
        end

        if me:mp() < mp then
            me:message('마력이 부족합니다.')
            return false
        end
        me:mp_down(mp)

        local success = math.random() < 0.3
        if not debug() and not success then
            return me:message('실패')
        end

        if spell.buff_cast(me, me, sp, {mp=0, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    ON_BUFF = function(me, sp)
        me:invincible(true)
    end,

    ON_UNBUFF = function(me, sp)
        me:invincible(false)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
