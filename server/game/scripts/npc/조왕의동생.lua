function NPC_183(me, npc)
    local quest = me:quest(QUEST_JOWANG)

    if quest == nil then
        ::NPC_183_0001::
        local btn = me:dialog(npc, '아.. 이 근처에 있다고 했는데... 어디있는 것일까...', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_183_0002::
        btn = me:dialog(npc, '아, 반갑네 내가 지금 뭘 좀 찾고 있느라 말이지. 실은 아내가 아이를 가졌는데 화기삼동초돈유합을 너무 먹고 싶다고 하는구만.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_183_0001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_183_0003::
        btn = me:dialog(npc, '그래서 여기저기서 전해지는 요리법으로 내가 한번 만들어보려고 하는데 재료조차 구하기가 어렵군.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_183_0002
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '날 좀 도와주지 않겠는가?', { '물론입니다. 부인의 몸조리에나 신경쓰십시오.', '죄송합니다만 그런일까지 할 시간이 없군요.' }, false)
        if sel == nil or sel ~= 0 then
            me:dialog(npc, '그렇다면 할 수 없군요..', false, true)
            return
        end
        local q = me:start_quest(QUEST_JOWANG)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        me:push_achievement(31, '조왕의동생의 부탁을 들어주자.', 7, 1)
        me:dialog(npc, '그럼 자네만 믿고 있겠네. 장안성의 푸줏간에 그 음식에 대해 잘 알고 있는 사람이 있다는 얘길 들은적이 있는데.. 혹시 참고가 될지 모르겠군.', false, true)
        return
    end

    if quest:completed() then
        me:dialog(npc, '저번엔 정말 감사했소!', false, true)
        return
    end

    local btn
    if not me:has_items('화기삼동충초돈유합', 1) then
        ::NPC_183_0005::
        btn = me:dialog(npc, '아직 화기삼동충초돈유합을 구하지 못하셨나보군요.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '장안성의 푸줏간에 그 음식에 대해 잘 알고 있는 사람이 있다네. 참고가 될지 모르겠군.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_183_0005
        end
        return
    end
    ::NPC_183_0007::
    btn = me:dialog(npc, '오! 드디어 구해왔군! 수고햇네. 어려운 부탁을 들어줘서 정말 고맙네. 이건 내 작은 정성이니 받아주게.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, '이걸 먹으면 자네의 체력이 더 강해질 걸세! 그럼 고맙네.. 잘가게나.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local code = me:exchange(
        { ['item'] = { ['화기삼동충초돈유합'] = 1 } },
        { ['item'] = { ['자양강장요리'] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '아직 화기삼동충초돈유합을 구하지 못하셨나보군요.', false, true)
        return
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 자양강장요리를 받을 수 없습니다.', false, true)
        return
    end
    quest:complete()
    me:push_achievement(31, '화기삼동충초돈유합을 전해주었다.', 6, 1)
end
