function NPC_122(me, npc)
    local quest = me:quest(QUEST_TONGTONG)
    local lighthouse = me:quest(QUEST_LIGHTHOUSE)

    if lighthouse == nil or lighthouse:completed() or lighthouse:step() ~= 3 then
        me:dialog(npc, '....', false, false)
        return
    end

    if quest == nil then
        local btn = me:dialog(npc, '안녕하신가, 나는 무기 수집가라네. 자네 대륙 사람 맞지?', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, lb = me:list(npc, '대륙에 망치라는 도구가 있다던데 쓸모가 많다더군. 혹시 하나 구해다 줄 수 있나?', { '네, 구해드리겠습니다.', '글쎄요...' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        if not me:start_quest(QUEST_TONGTONG) then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        me:dialog(npc, '이 일기같은건 잘 보관하고 있을테니, 조심해서 다녀오시게.', false, true)
        return
    end

    if quest:completed() then
        me:dialog(npc, '흠..이 단단하고 묵직한 망치. 정말 쓸 곳이 많구만.', false, true)
        return
    end

    if not me:has_items('망치', 1) then
        me:dialog(npc, '아직 망치라는 것을 구하지 못한것 같군?', false, true)
        return
    end

    local btn = me:dialog(npc, '과연! 묵직하고 단단한 것이 무기로도 손색이 없겠어!', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    if not me:rmitem('망치', 1, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, '아직 망치라는 것을 구하지 못한것 같군?', false, true)
        return
    end
    if me:mkitem('선장의일기3', 1) == nil then
        me:dialog(npc, '소지품이 가득 차서 선장의일기3을 줄 수 없네.', false, true)
        return
    end
    quest:complete()
    lighthouse:step(4)
    me:push_achievement(32, '진백랑의 부탁을 들어주자. [4/6]', 7, 1)
    me:dialog(npc, '이건 약속했던 누군가의 일기일세! 잘 가시게!', false, true)
end