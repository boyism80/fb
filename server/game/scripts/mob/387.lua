-- mob: 달걀귀신
local quest = require('lib.quest')

return {
    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_die = function(me)
    -- end,

    on_mob_kill = function(me, mobs)
        quest.king_on_mob_kill(me, mobs)
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
