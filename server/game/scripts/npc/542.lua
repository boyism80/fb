-- npc: 영환도사
local quest = require('lib.quest')
local server = require('lib.server')
function NPC_542(me, npc)
    local q = me:quest(quest.QUEST_JINHWANG)
    if q == nil then
        return
    end
    if q:completed() then
        return
    end

    local step = q:step()
    local button

    if step == 6 then
        ::NPC_542_0000::
        button = me:dialog(npc, "무슨일로 나를 찾아왔는가? 난 방금 여행에서 돌아와 무척 피곤하다네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_542_0001::
        button = me:dialog(npc, "혈황검? 글쎄. 난 그런검을 들어보지 못했는데..", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0000
        end

        ::NPC_542_0002::
        button = me:dialog(npc, "음.. 혈황검이란 것이 중국황실의 보물인 진황보검의 반쪽이란 말이지. 나야 원래 재물에 관심이 없으니 혈황검에 대한 소식을 듣게 되면 자네에게 알려주겠네.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0001
        end

        ::NPC_542_0003::
        button = me:dialog(npc, "그런데 내가 여행을 간 사이 강시들이 엄청나게 늘어나서 진시황릉을 차지하고 있더군. 이미 너무 많이 불어나서 나 혼자만으론 역부족이라네.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0002
        end

        ::NPC_542_0004::
        local sel, list_btn = me:list(npc, "강시를 잡는 일을 도와주겠는가?", {
            "좋습니다! 기꺼이 도와드리죠.",
            "강시를? 너무 어려운 부탁입니다.",
        }, true)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_542_0003
        end
        if sel == nil or sel ~= 0 then
            return
        end

        ::NPC_542_0005::
        button = me:dialog(npc, "고맙네. 내 부탁만 제대로 들어준다면 혈황검을 찾는 일을 도와주도록 하겠네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_542_0006::
        button = me:dialog(npc, "잘듣게. 강시는 정말 강한 상대라네. 특히 강시의 왕인 만년원귀는 누구 혼자 물리칠 수 있는 상대가 아니야. 우선 자네의 강함을 시험하겠네.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0005
        end

        ::NPC_542_0007::
        button = me:dialog(npc, "강시들을 물리쳐 강시의손톱을 모아와주게. 100개의 강시의 손톱을 모아와보도록 하게나. 만년원귀를 바로 상대했다가 개죽음을 당할 수도 있지 않는가.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0006
        end

        me:push_achievement(511, "진황보검을 찾아서 (강시를 처리하자)", 7, 20)
        q:step(7)
        return
    end

    if step == 7 then
        if not me:has_items("강시의손톱", 100) then
            button = me:dialog(npc, "강시들을 물리쳐 강시의손톱을 모아와주게. 100개의 강시의 손톱을 모아와보도록 하게나. 만년원귀를 바로 상대했다가 개죽음을 당할 수도 있지 않는가.", false, true)
            return
        end

        if not me:rmitem({ ["강시의손톱"] = 100 }, ITEM_DELETE_TYPE.GIVE) then
            if not me:has_items("강시의손톱", 100) then
                me:dialog(npc, "강시의손톱 100개가 없지 않은가.", false, false)
            else
                me:dialog(npc, "강시의손톱이 없지 않은가.", false, false)
            end
            return
        end

        q:step(8)

        ::NPC_542_0008::
        button = me:dialog(npc, "수고했네. 해내리라 믿었다네. 그럼 이제 강시의 왕을 처치해야겠군. 자네도 짐작했을텐데. 강시는 우두머리를 제거하면 힘이 약해져 모두 흩어지지.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_542_0009::
        button = me:dialog(npc, "바로 만년원귀가 그들의 우두머리네. 만년원귀를 제거하고 그의 손톱을 징표로 가져다주게나.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0008
        end

        ::NPC_542_0010::
        button = me:dialog(npc, "그럼 조심하고 꼭 성공하길 빌겠네!", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0009
        end
        return
    end

    if step ~= 8 then
        return
    end

    if not me:has_items("만년원귀의손톱", 1) then
        ::NPC_542_0018::
        button = me:dialog(npc, "수고했네. 해내리라 믿었다네. 그럼 이제 강시의 왕을 처치해야겠군. 자네도 짐작했을텐데. 강시는 우두머리를 제거하면 힘이 약해져 모두 흩어지지.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_542_0019::
        button = me:dialog(npc, "바로 만년원귀가 그들의 우두머리네. 만년원귀를 제거하고 그의 손톱을 징표로 가져다주게나.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_542_0018
        end
        return
    end

    local code = me:exchange(
        { ['item'] = { ["만년원귀의손톱"] = 1 } },
        { ['item'] = { ["혈황검"] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "만년원귀의손톱이 없지 않은가.", false, false)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 줄 수 없네.", false, false)
        return
    end

    me:push_achievement(511, "진황보검을 찾아서 (혈황검을 찾았다)", 7, 20)
    q:step(9)

    ::NPC_542_0011::
    button = me:dialog(npc, "해냈군! 드디어 만년원귀를.. 내 나이가 들어 상대를 하지 못하는게 한이었는데. 자네가 날 대신해서 만년원귀를 물리쳐줬군", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_542_0012::
    button = me:dialog(npc, "그런데 혈황검이란 것은 어떻게 생긴 검인가? 설명이라도 해주게나.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_542_0011
    end

    ::NPC_542_0013::
    button = me:dialog(npc, "음.. 음.. 내가 가진 검과 비슷하게 생긴것인가보군?", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_542_0012
    end

    ::NPC_542_0014::
    button = me:dialog(npc, "엇. 이 칼의 귀퉁이에 조그맣게 글자가 써있는데... 아니 이것이 혈황검이었군! 등잔밑이 어둡다더니..", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_542_0013
    end

    ::NPC_542_0015::
    button = me:dialog(npc, "그럼 이걸 자네에게 주도록 하지. 나에게 필요한 물건도 아니니..", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_542_0014
    end

    ::NPC_542_0016::
    button = me:dialog(npc, "그럼 황실에 안부라도 전해주시게나. 허허허.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_542_0015
    end
end
