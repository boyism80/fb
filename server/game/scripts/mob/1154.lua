-- mob: 쾌도해골
-- 쾌도해골 사망
local quest = require('lib.quest')

function ON_MOB_KILL_1154(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_1154(me)
-- end
