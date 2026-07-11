-- mob: 해마장군
local quest = require('lib.quest')

function ON_MOB_KILL_1086(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 7 then
        return
    end

    q:param("1")
    me:push_achievement(19, "해마장군을 생포하였다!", 7, 1)
end

-- function ON_MOB_DIE_1086(me)
-- end

-- 해마장군 공격
-- function ON_MOB_ATTACK_1086(me, you)
--
-- end
