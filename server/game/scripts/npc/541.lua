-- npc: 북천좌상
local quest = require('lib.quest')
local enum = require('lib.enum')
function NPC_541(me, npc)
    local q = me:quest(quest.QUEST_JINHWANG)
    if q == nil then
        return
    end

    local step = q:step()

    if step == 1 then
        ::NPC_541_0000::
        local button = me:dialog(npc, "진황보검에 대해서 얘기하는건가? 자네 그건 어디서 들은건가! 진황보검에 대해서 언급하는 자는 모두 하옥시키라고 하셨네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_541_0001::
        button = me:dialog(npc, "여봐라! 이자를 당장 하옥시켜라~", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_541_0000
        end

        local map = name2map("장안성감옥")
        if map then
            me:map(map, { math.random(2, 3), 5 })
        end
        return
    end

    if step ~= 9 then
        return
    end

    local code = me:exchange(
        { ['item'] = { ["진랑검"] = 1, ["혈황검"] = 1 } },
        { ['item'] = { ["강철손목보호대"] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        if not me:has_items("진랑검", 1) or not me:has_items("혈황검", 1) then
            me:dialog(npc, "퀘스트 오류입니다.\n\n운영자에게 문의하세요.\n(*진랑검, 혈황검이 없을 경우 나타나는 메시지 입니다.)", false, false)
        else
            me:dialog(npc, "진랑검과 혈황검이 없지 않은가.", false, false)
        end
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 줄 수 없네.", false, false)
        return
    end

    q:step(10)

    local button
    ::NPC_541_0010::
    button = me:dialog(npc, "정말 대단한일을 해냈군 자네. 이 진황보검이 제자리를 찾아 중국 황실도 다시 평온해질걸세. 이것은 우리 황실 모두의 보답이네.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_541_0011::
    button = me:dialog(npc, "언제 한번 다시 황궁에 들리게. 나와 같이 술이나 한잔하세. 진황보검을 찾으면서 일어났던 일들도 꼭 듣고싶네. 허허~", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_541_0010
    end

    ::NPC_541_0012::
    button = me:dialog(npc, "그럼 잘 지내고 언제나 의협심을 잊지 말게나.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_541_0011
    end
end
