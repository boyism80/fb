-- mob: 문어장군
local quest = require('lib.quest')

return {
    -- ON_MOB_ATTACK = function(me, you)
    -- end,

    -- ON_MOB_DIE = function(me)
    -- end,

    ON_MOB_KILL = function(me, mobs)
        if me == nil or mobs == nil or #mobs == 0 then
            return
        end
        local mob = mobs[1]
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local q = me:quest(quest.QUEST_DRAGON_KING)
        if q == nil or q:step() ~= 5 then
            return
        end

        local sel, list_btn = me:list(mob, "으으윽...", { "용왕님의 부탁으로 네놈을 잡으러 왔다" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end

        sel, list_btn = me:list(mob, "아니 용왕님이 왜 나를 잡아 오라했단 말이냐?", { "네놈이 반란에 연루되어 있다고", "게장군이 다 털어놓았다.", "이제와서 발뺌하려 해도 소용없는 짓이야." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 1 or sel > 3 then
            return
        end

        sel, list_btn = me:list(mob, "반란이라니!! 나는 반란에 가담한적이 없어!!", { "후후..과연 용왕님이 그 말을 믿어주실까?", "나도 못 믿겠는데,", "어찌 용왕님이 그 말을 믿어 주신단 말이냐?" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 1 or sel > 3 then
            return
        end

        sel, list_btn = me:list(mob, "아..억울하구나. 억울해. 게장군이 나에게 무슨 원한이 있어서", { "..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end

        sel, list_btn = me:list(mob, "나를 끌어 들였단 말이냐. 이 일을 어찌할꼬.. 이 일을..", { "나에게 하소연 해 봐야 소용 없는 짓,", "조용히 용왕님에게 가자." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 1 or sel > 2 then
            return
        end

        sel, list_btn = me:list(mob, "아..나의 충성심을 몰라 주시는 용왕님이 너무나도 야속하구나.", { "..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end

        sel, list_btn = me:list(mob, "신하가 주군의 믿음을 받지 못하면 살아 있을 가치가 없는 법.", { "..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end

        sel, list_btn = me:list(mob, "나의 결백함을 증명해 보여 용왕님에 대한 나의 충성을 보이겠다.", { "..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end

        sel, list_btn = me:list(mob, "이보게, 나의 이 마음을 다문창과 함께 꼭 용왕님께 전해 주시게.", { "이런, 무슨 짓을....?" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end

        q:progress(1)
        me:push_achievement(19, "문어장군을 생포하였다!", 7, 1)
    end,

    -- ON_MOB_SPELL_HIT = function(me, you, spell)
    -- end
}
