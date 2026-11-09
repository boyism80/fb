-- mob: 녹살쾡이
local quest = require('lib.quest')

return {
    on_mob_kill = function(me, mobs)
        quest.king_on_mob_kill(me, mobs)
    end,
}
