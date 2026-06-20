-- 순제 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1163(me, you)
    return false
end

-- 순제 사망
function ON_MOB_DIE_1163(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    local q = you:quest(quest.QUEST_MUTA)
    if q == nil or q:step() ~= 5 then
        return
    end
    you:mkitem('순제의증표', 1)
end