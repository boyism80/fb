-- mob: 범천


return {
    on_mob_action = function(me, you)
        return false
    end,


    on_mob_die = function(me)
        me:chat('크롸롸!!', CHAT_TYPE.NORMAL, true)
    end

    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_kill = function(me, mobs)
    -- end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end,
}
