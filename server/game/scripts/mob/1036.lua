-- mob: 선비평민
local quest = require('lib.quest')

return {
    ON_MOB_ATTACK = function(me, you)
        return false
    end,

    -- ON_MOB_DIE = function(me)
    -- end,

    ON_MOB_KILL = function(me, mobs)
        quest.king_on_mob_kill(me, mobs)
    end,

    -- ON_MOB_SPELL_HIT = function(me, you, spell)
    -- end
}
