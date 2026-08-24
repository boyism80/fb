-- mob: 폭염왕


return {
    on_mob_action = function(me, you)
        return false
    end,


    on_mob_die = function(me)
        me:chat('폭염왕: 크아아악!! 아직 주어진 사명을 다하지 못했건만...', CHAT_TYPE.NORMAL, false)
    end

    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_kill = function(me, mobs)
    -- end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end,
}
