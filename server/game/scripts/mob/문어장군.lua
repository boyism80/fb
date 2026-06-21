local quest = require('lib.quest')

function ON_MOB_DIE_1084(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = you:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 5 then
        return
    end

    local sel, list_btn = you:list(me, "으으윽...", { "용왕님의 부탁으로 네놈을 잡으러 왔다" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = you:list(me, "아니 용왕님이 왜 나를 잡아 오라했단 말이냐?", { "네놈이 반란에 연루되어 있다고", "게장군이 다 털어놓았다.", "이제와서 발뺌하려 해도 소용없는 짓이야." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 2 then
        return
    end

    sel, list_btn = you:list(me, "반란이라니!! 나는 반란에 가담한적이 없어!!", { "후후..과연 용왕님이 그 말을 믿어주실까?", "나도 못 믿겠는데,", "어찌 용왕님이 그 말을 믿어 주신단 말이냐?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 2 then
        return
    end

    sel, list_btn = you:list(me, "아..억울하구나. 억울해. 게장군이 나에게 무슨 원한이 있어서", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = you:list(me, "나를 끌어 들였단 말이냐. 이 일을 어찌할꼬.. 이 일을..", { "나에게 하소연 해 봐야 소용 없는 짓,", "조용히 용왕님에게 가자." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end

    sel, list_btn = you:list(me, "아..나의 충성심을 몰라 주시는 용왕님이 너무나도 야속하구나.", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = you:list(me, "신하가 주군의 믿음을 받지 못하면 살아 있을 가치가 없는 법.", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = you:list(me, "나의 결백함을 증명해 보여 용왕님에 대한 나의 충성을 보이겠다.", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = you:list(me, "이보게, 나의 이 마음을 다문창과 함께 꼭 용왕님께 전해 주시게.", { "이런, 무슨 짓을....?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    q:progress(1)
    you:push_achievement(19, "문어장군을 생포하였다!", 7, 0)
end

-- 문어장군 공격
function ON_MOB_ATTACK_1084(me, you)

end