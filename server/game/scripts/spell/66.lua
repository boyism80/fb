-- spell: 다리밟기
-- Buff from 달맞이다리 / 오작교: periodic HP/MP regen while active.

return {
    on_cast = function(me, you, sp)
        local target = you or me
        if target == nil then
            return false
        end
        target:buff(sp, 600, me)
        me:message("다리밟기 외웠습니다.")
        return true
    end,

    on_buff = function(me, sp)
    end,

    on_unbuff = function(me, sp)
    end,

    on_concast = function(me, sp)
        me:effect(133)
        local maxhp = me:maxhp()
        local heal_hp = math.max(1, maxhp // 100)
        if me:hp() < maxhp - heal_hp then
            me:hp(me:hp() + heal_hp)
        end

        local maxmp = me:maxmp()
        local heal_mp = math.max(1, maxmp // 100)
        if me:mp() < maxmp - heal_mp then
            me:mp(me:mp() + heal_mp)
        end
    end
}
