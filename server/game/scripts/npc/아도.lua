
function NPC_447(me, npc)
    local quest = me:quest(QUEST_BEGINNER_PATH)
    if quest == nil then
        me:dialog(npc, "먼저 앞의 도우미 분들의 도움을 받고 오세요.", false, false)
        return
    end
    if quest:completed() then
        me:dialog(npc, "이제 직업의길로 가보시면 됩니다.", false, false)
        return
    end
    local step = quest:step()
    if step < 7 then
        me:dialog(npc, "먼저 앞의 도우미 분들의 도움을 받고 오세요.", false, false)
        return
    end

    if step == 7 then
        local btn = me:dialog(npc, "안녕하십니까. 저는 아도라고 합니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "저는 아이템을 다루는 방법을 알려드리고자 합니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "먼저 아이템을 사용 하는 방법을 알려드리겠습니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, me:name() .. "님의 체력을 절반으로 깎을테니, 제가 드린 아이템 [웅담]을 사용하여 보세요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "아이템을 사용하시려면, 목도를 착용했듯 알파벳 i를 눌러 인벤토리를 열고, 해당 아이템을 클릭하시면 됩니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if me:mkitem("웅담", 2) == nil then
            me:dialog(npc, "소지품이 가득 차서 웅담을 줄 수 없습니다.", false, false)
            return
        end
        quest:step(8)
        me:dialog(npc, "웅담 2개 받았다.", false, false)
        return
    end

    if step == 8 then
        local full_hp = (me:hp() and me:maxhp() and me:hp() >= me:maxhp())
        if not full_hp then
            me:dialog(npc, "인벤토리는 i를 이용해 열 수 있고, [웅담]을 더블클릭하면 아이템을 사용하실 수 있습니다.", false, true)
            me:dialog(npc, "혹은 u를 누르고, 사용할 아이템의 알파벳을 입력하거나, 혹은 z키를 누르고 사용할 아이템을 지정하실 수도 있습니다.", false, true)
            me:dialog(npc, "웅담을 사용해 체력을 가득 채워 보세요.", false, false)
            return
        end
        me:dialog(npc, "잘 하셨습니다! 이제 훌륭하게 아이템을 사용하실 수 있으시군요.", false, true)
        me:dialog(npc, "그렇다면 이번에는 [조합]을 하는 방법을 알려 드리겠습니다. 조금 어려워도 잘 따라 오실 수 있으실겁니다.", false, true)
        me:dialog(npc, "이번에는 제가 [동동주]와 [막걸리]를 드릴테니, 해당 아이템을 조합 해 보도록 하겠습니다.", false, true)
        me:dialog(npc, "조합을 할 준비가 되셨으면, 다시 저를 클릭 해 주세요.", false, true)
        if me:mkitem("동동주", 1) == nil or me:mkitem("막걸리", 1) == nil then
            me:dialog(npc, "소지품이 가득 찼습니다.", false, false)
            return
        end
        quest:step(9)
        me:dialog(npc, "동동주 1개, 막걸리 1개 받았다.", false, false)
        return
    end

    if step == 9 then
        if me:has_items("탁주", 1) then
            me:dialog(npc, "훌륭합니다! 조합까지 완벽히 하셨으니, 이제 다음 단계로 가셔도 문제가 없겠네요.", false, true)
            me:dialog(npc, "이제 직업의길로 보내드릴테니, 어떤 직업을 선택하실지 곰곰히 생각해 보시길 바랄게요. 그럼 안녕히~!", false, true)
            quest:step(10)
            me:exp(me:exp() + 600)
            me:mkitem("노란비서", 21)
            me:mkitem("직업의길비서", 1)
            local map = name2map("직업의길")
            if map then
                me:map(map, math.random(14, 16), math.random(14, 16))
            end
            return
        end
        if not me:has_items("동동주", 1) or not me:has_items("막걸리", 1) then
            me:dialog(npc, "저런, 동동주와 막걸리를 잃어버리셨나요? 그럼 제가 하나를 다시 드릴테니...", false, true)
            me:dialog(npc, "도토리 [50]개만 구해오세요. 그러면 동동주와 막걸리를 하나씩 드릴게요.", false, true)
            local code = me:exchange(
                { ['item'] = { ["도토리"] = 50 } },
                { ['item'] = { ["동동주"] = 1, ["막걸리"] = 1 } }
            )
            if code == EXCHANGE_RESULT.OK then
                me:dialog(npc, "동동주와 막걸리를 하나씩 드렸으니, 이제 잃어버리지 않도록 조심하세요.", false, false)
            elseif code == EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, "도토리가 부족합니다.", false, false)
            elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 동동주와 막걸리를 줄 수 없습니다.", false, false)
            end
            return
        end
        me:dialog(npc, "자, 이번에 알려드릴 것은 [조합]이라는 건데요...! 어디 한번 알아보도록 하겠습니다.", false, true)
        me:dialog(npc, "제가 드린 동동주와 막걸리를 조합해서 [탁주]라는 아이템을 만들건데요,", false, true)
        me:dialog(npc, "[쉬프트(SHIFT)]와 [알파벳 i]를 동시에 클릭해 조합창을 띄우고, 그 곳에 동동주와 막걸리를 추가해 조합하시면 됩니다.", false, true)
        me:dialog(npc, "조합을 완료하시면 다시 저를 클릭 해 주세요!", false, false)
    end
end
