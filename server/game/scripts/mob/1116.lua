-- mob: 용왕용천
local dragon_weapon = require('lib.dragon_weapon')

return {
    on_mob_attack = function(me, you)
        return false
    end,

    -- on_mob_die = function(me)
    -- end,

    on_mob_kill = function(me, mobs)
        dragon_weapon.on_king_kill(me, mobs, '용천제일검')
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
