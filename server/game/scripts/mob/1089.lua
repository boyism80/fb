-- mob: 인어장군
local quest = require('lib.quest')

function ON_MOB_KILL_1089(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 9 then
        return
    end

    local sel, list_btn = me:list(mob, "네...네놈은 누구냐?", { "용왕님의 부탁을 받고", "너를 사로 잡아 용왕님께 데려가려 왔다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end

    sel, list_btn = me:list(mob, "요..용왕님께서 나를?", { "그래, 네가 반란을 꾀한 사실을", "용왕님은 다 알고 계신다.", "또, 다른 장군들을 반란에", "끌어 들인 사실도 알고 계신다", "용왕님 앞으로 가서 모든걸 털어놓도록 해라." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 4 then
        return
    end

    local btn = me:dialog(mob, "아.. 이 일을 어쩌면 좋단 말이야.. 이 일을..\n여보시오, 용왕님께서 내 죄를 모두 알고 계신다고 하니 본인이 부정한들 무슨 소용이 있겠소만, 지금은 본인의 어미가 많이 아프셔서 그곳에 가 봐야 한다오.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    sel, list_btn = me:list(mob, "그러니, 며칠 후 내가 용왕님께 직접 찾아 뵈올 테니 오늘은 그냥 돌아가시면 안되오?", { "흥! 그 말을 내가 어떻게 믿으라는 소리냐?", "또, 용왕님께는 뭐라 말하고?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end

    sel, list_btn = me:list(mob, "그동안 반란에 가담하였던 자들과 중요한 내용이 적힌 내통문서를 줄 터이니,", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, "이것을 용왕님에게 전해주면 될 것이오. 부탁이오", { "좋다. 이리 가지고 오너라.", "아니 절대 믿을 수 없다!" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, "여기 내통문서가 있소.", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end

    sel, list_btn = me:list(mob, "내 어미를 돌보고 바로 용왕님께 찾아갈터이니 그만 놓아 주시오.", { "알겠다. 내통문서도 받았으니 우선 이것을", "용왕님께 전해주겠다.", "네 어미를 돌보고 반드시 용왕님을 찾아 오너라" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 2 then
        return
    end

    q:progress(1)
    if not me:has_items("내통문서", 1) then
        me:mkitem("내통문서", 1)
        me:push_achievement(19, "내통문서를 얻다.", 7, 0)
    end
end

function ON_MOB_DIE_1089(me)
end

-- 인어장군 공격
function ON_MOB_ATTACK_1089(me, you)

end
