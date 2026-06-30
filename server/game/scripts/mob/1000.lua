-- mob: 복어장군
local quest = require('lib.quest')

function ON_MOB_DIE_1000(me, you)
    me:chat('asd')
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = you:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 1 or q:progress() < 1 then
        return
    end
    local item_doc = "태자전음문서"
    if you:has_items(item_doc, 1) then
        return
    end

    you:mkitem(item_doc, 1)
    you:push_achievement(19, "복어장군을 생포하였다!", 7, 0)
end

-- 복어장군 공격
function ON_MOB_ATTACK_1000(me, you)

end
