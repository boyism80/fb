function ON_MOB_DIE_1092(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local quest = you:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end

    local step = quest:step()
    if step == 12 and quest:progress() == 0 then
        local sel, list_btn = you:list(me, "으윽... 비록 나는 잡힐지언정 우리 군사들의 사기는 절대 꺽이지 않을 것이다.", { "닥쳐라", "어서 나를 따라서 용왕님을 뵈러 가자." }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
            return
        end
        quest:progress(1)
        you:push_achievement(19, "상어장군을 생포하였다. [1/2]", 7, 0)
        return
    end

    if step ~= 13 then
        return
    end

    local anger_end = tonumber(quest:param() or "")
    if not anger_end or now() < anger_end then
        return
    end

    local sel, list_btn = you:list(me, "한번만 눈감아 주시면 평생 쓸만한 금은보화를 드리겠습니다.", { "으음...그래 좋다.", "나는 금은보화 따위에 넘어가지 않는다!" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        quest:progress(2)
        you:mkitem("금은보화", 1)
        you:push_achievement(19, "상어장군을 놓아주다.", 7, 0)
        you:dialog(me, "감사합니다. 자 이걸 받아주십시오. 그럼 전 사라지겠습니다.", false, true)
    else
        quest:progress(1)
        you:push_achievement(19, "상어장군을 생포하였다. [2/2]", 7, 0)
    end
end

-- 상어장군 공격
function ON_MOB_ATTACK_1092(me, you)

end