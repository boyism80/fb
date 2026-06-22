-- mob: 해파리부하
local quest = require('lib.quest')

function ON_MOB_DIE_1007(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = you:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 16 then
        return
    end

    if math.random(1, 100) > 10 then
        return
    end

    if you:has_items("전략문서", 1) then
        return
    end

    local item = you:mkitem("전략문서", 1)
    you:dialog(item, "휴.. 드디어 전략문서를 구했다.", false, true)
end
