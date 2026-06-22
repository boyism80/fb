-- mob: 소호
-- 소호 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1173(me, you)
    return false
end

-- 소호 사망
function ON_MOB_DIE_1173(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    local q = you:quest(quest.QUEST_MUTA)
    if q == nil or q:step() ~= 1 then
        return
    end
    you:mkitem('소호의증표', 1)
end
