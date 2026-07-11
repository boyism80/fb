-- mob: 반고
-- 반고 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1162(me, you)
    return false
end

-- 반고 사망
function ON_MOB_KILL_1162(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_MUTA)
    if q == nil or q:step() ~= 6 then
        return
    end
    me:mkitem('반고의심장', 1)
end

function ON_MOB_DIE_1162(me)
end
