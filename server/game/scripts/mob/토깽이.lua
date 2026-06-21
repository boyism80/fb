local quest = require('lib.quest')

local ACHIEVEMENT_RABBIT_CAPTURE = 20
local ITEM_RABBIT_LIVER = '토끼의간'

function ON_MOB_DIE_37(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = you:quest(quest.QUEST_RABBIT_LIVER)
    if q == nil or q:step() ~= 2 then
        return
    end
    if you:has_items({ [ITEM_RABBIT_LIVER] = 1 }) then
        return
    end

    local sel, list_btn = you:list(me, '여보시오. 여보시오. 왜 나를 때려 잡으려 한 단 말이오?', { '나는 네 간이 필요하다' }, false)
    if sel == nil or sel ~= 0 then
        return
    end

    local msg2 = string.format('하하하.. 여보시오 %s님, 내 간이 얼마나 중요한 것인데 그것을 가지고 다닌단 말이오?', you:name())
    sel, list_btn = you:list(me, msg2, { '그렇다면 네 간은 지금 어디에 있단 말이냐?' }, false)
    if sel == nil or sel ~= 0 then
        return
    end

    local btn = you:dialog(me, '당연히 아무도 모르는 곳에 잘 숨겨 두었소. 음.. 하지만 다른분도 아닌 용왕님의 병이 위중하시다 하니, 내 기꺼이 내 간을 용왕님께 드리리다.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    sel, list_btn = you:list(me, '어떻소?', { '좋아, 그럼 내 여기서 기다리마', '흥, 내가 속을줄 아느냐?' }, false)
    if sel == nil then
        return
    end
    
    if sel ~= 1 then
        return
    end

    you:mkitem(ITEM_RABBIT_LIVER, 1)
    you:push_achievement(ACHIEVEMENT_RABBIT_CAPTURE, '토깽이를 성공적으로 포획했다!', 7, 0)
    you:dialog(me, '으으윽..', false, false)
end