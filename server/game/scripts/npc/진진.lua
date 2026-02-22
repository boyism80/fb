function NPC_118(me, npc)
    local quest = me:quest(QUEST_JINJIN)
    local lighthouse = me:quest(QUEST_LIGHTHOUSE)

    if quest == nil then
        if lighthouse == nil or lighthouse:completed() or lighthouse:step() ~= 1 then
            goto NPC_118_DEFAULT
        end
        local item = me:item('초보도시락')
        if item == nil then
            goto NPC_118_DEFAULT
        end
        local sel, lb = me:list(npc, '잠깐만! 혹시 여유가 있다면 그 도시락을 나에게 주시지 않겠소?', { '네, 그러지요.', '주기 싫은데요.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        if not me:start_quest(QUEST_JINJIN) then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest = me:quest(QUEST_JINJIN)
        if me:mkitem('선장의일기1', 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 선장의일기1을 줄 수 없네.', false, true)
            return
        end
        me:rmitem('초보도시락', 1, ITEM_DELETE_TYPE.GIVE)
        quest:complete()
        lighthouse:step(2)
        me:push_achievement(32, '진백랑의 부탁을 들어주자. [2/6]', 7, 1)
        me:dialog(npc, '고맙소! 일이 급해서 도시락 만들 여유가 없었거든! 자, 이거라도 받으시오!', false, true)
        return
    end

    ::NPC_118_DEFAULT::
    me:dialog(npc, '일을 가야하는데 도시락을 깜빡 잊었네. 이를 어쩐담?! 이제와서 만들 시간도 없는데. 누가 초보 도시락이라도 팔지 않으려나.', false, true)
end