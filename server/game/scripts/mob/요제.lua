-- 요제 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1164(me, you)
    return false
end

-- 요제 사망
function ON_MOB_DIE_1164(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    local q = you:quest(quest.QUEST_MUTA)
    if q == nil or q:step() ~= 4 then
        return
    end
    you:mkitem('요제의증표', 1)
end