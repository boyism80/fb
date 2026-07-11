-- mob: 해파리장군
local quest = require('lib.quest')

function ON_MOB_KILL_1093(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 15 or q:progress() ~= 0 then
        return
    end

    local sel, list_btn = me:list(mob, "네놈은 뭍사람인데 어찌하여 용왕님의 편을 드는 것이냐?", { "아니면 반란군인 네놈 편을 들란 말이냐?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, "뭍사람이 함부로 용궁의 일에 간섭하지 말고 썩 꺼져라", { "이미 사로잡힌 주제에 말이 많구나." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, " ", { "내 하나만 물어보겠다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, " ", { "네놈들의 주동자가 누구이더냐?", "필시 전략문서가 있을 터, 그것이 어디 있느냐?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end

    sel, list_btn = me:list(mob, "그것만큼은 절대로 말해줄 수 없다.", { "어서 말하지 못할까!!" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, "흥. 그런다고 내가 겁먹을 것 같으냐.", { "정말 대단한 악질이로구나." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    q:progress(1)
    me:push_achievement(19, "해파리장군을 생포하였다.", 7, 0)
end

function ON_MOB_DIE_1093(me)
end

-- 해파리장군 공격
function ON_MOB_ATTACK_1093(me, you)

end
