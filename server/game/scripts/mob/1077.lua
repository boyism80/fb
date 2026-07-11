-- mob: 복홍
local quest = require('lib.quest')

function ON_MOB_KILL_1077(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 1 then
        return
    end

    local sel, list_btn = me:list(mob, ' ', { '네놈이 복어대장이냐?' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, '아닙니다. 저는 복어대장의 수하일뿐입니다.', { '좋다. 내 질문에 한치의 거짓없이 ' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, '아닙니다. 저는 복어대장의 수하일뿐입니다.', { '대답을 하면 네놈을 살려주겠다.' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, '예.....', { '복어대장이라는 자는 어디에 있느냐?' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, '글쎄요... 워낙 지체가 높으신 분이라 저도 확실히는 모르겠으나, 사방이 막혀있고 입구가 좋은 곳에 살고 있다 들었습니다.', { '좋다. 또 묻겠다.' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, ' ', { '복어대장이라는 자가 용왕님에게 반기를' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, ' ', { '들려 한다고 하는데, 필시 그에 공조하는' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, ' ', { '무리들이 있을 것이다.' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, ' ', { '그자들이 누구냐?' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, '소인이 어떻게 그런것을 알겠습니까? 복어대장이라....읍...', { '왜 말을 하려다 마는 것이냐?' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, ' ', { '네 이놈! 살고 싶지 않느냐?' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, '아예..예.. 복어대장은 그 긴 소매에 항시 중요한 것들을 숨기고 다닌다 합니다. 거기에 뭔가 중요한 문서가 있지 않을까 합니다.', { '음 그럴듯 하군.' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, ' ', { '알겠다. 그만 사라지거라.' }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    q:progress(1)
end

function ON_MOB_DIE_1077(me)
end
