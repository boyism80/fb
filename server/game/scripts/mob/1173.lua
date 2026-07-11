-- mob: 소호
-- 소호 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1173(me, you)
    return false
end

-- 소호 사망
function ON_MOB_KILL_1173(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_MUTA)
    if q == nil or q:step() ~= 1 then
        return
    end
    me:mkitem('소호의증표', 1)
end

function ON_MOB_DIE_1173(me)
end
