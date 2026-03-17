
function NPC_119(me, npc)
    local quest = me:quest(QUEST_DETECTIVE)
    if quest == nil or quest:completed() then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end
    local step = quest:step()

    if step == 13 then
        local sel, list_btn = me:list(npc, "진진 소식은 들었네... 가슴이 아프구먼.", {
            "다른 도움이 될 만한 건 없을까요?",
            "참, 요리 이야기 좀 해주세요.",
            "알 수 없는 세상이죠, 그럼 이만.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            ::NPC_119_0001::
            local btn = me:dialog(npc, "음, 진진이 대륙 식재료를 팔겠다길래. 한 밤 중에 잠도 못자고 바닷가로 나갔지 뭔가.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_119_0002::
            btn = me:dialog(npc, "한참 기다리니 배 한 척이 오고, 진진이 궤짝을 나르는 동안, 그 배 선원들에게서 식재료를 좀 샀었어.", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_119_0001
            end
            quest:step(14)
            me:dialog(npc, "그러고보니 배 이름이 참 묘했지. 은익...이라고 하더군. 숨은 날개라는 뜻도 가지고, 은밀한 이익이란 뜻도 가졌으니까 말야.", false, false)
        elseif sel == 1 then
            me:dialog(npc, "준비중입니다.", false, false)
        else
            me:dialog(npc, "그럼 이만.", false, false)
        end
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
