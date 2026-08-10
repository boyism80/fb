-- spell: 반짝반짝
-- Applied by 단오 머리감기 / 유두 길림성 폭포 목욕.

return {
    on_buff = function(me, sp)
    end,

    on_unbuff = function(me, sp)
    end,

    on_concast = function(me, sp)
        me:effect(130)

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
    end,
}
