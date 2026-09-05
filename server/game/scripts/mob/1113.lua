-- mob: 용왕용겸

local dragon_weapon = require('lib.dragon_weapon')

return {
    on_mob_action = function(me, you)
        return false
    end,



    on_mob_kill = function(me, mobs)
        dragon_weapon.on_king_kill(me, mobs, '용겸제일봉')
    end,

    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_die = function(me)
    -- end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end,
}
