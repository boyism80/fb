-- mob: 직녀
-- Movement / reunion dialogue is driven by lib/magpie_bridge.lua (map timer).

return {
    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_die = function(me)
    -- end,

    -- on_mob_kill = function(me, mobs)
    -- end,

    on_mob_spell_hit = function(me, you, spell)
        me:hp(me:maxhp())
        return false
    end
}
