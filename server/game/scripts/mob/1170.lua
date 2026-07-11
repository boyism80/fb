-- mob: 전욱
-- 전욱 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1170(me, you)
    return false
end

-- 전욱 사망
function ON_MOB_KILL_1170(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_MUTA)
    if q == nil or q:step() ~= 2 then
        return
    end
    me:mkitem('전욱의증표', 1)
end

-- function ON_MOB_DIE_1170(me)
-- end
