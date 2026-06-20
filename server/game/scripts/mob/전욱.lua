-- 전욱 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1170(me, you)
    return false
end

-- 전욱 사망
function ON_MOB_DIE_1170(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    local q = you:quest(quest.QUEST_MUTA)
    if q == nil or q:step() ~= 2 then
        return
    end
    you:mkitem('전욱의증표', 1)
end