function NPC_216(me, npc)
    local REWARD_PER_BAG = 5000

    local quest = me:quest(QUEST_RED_CLAY)

    if quest == nil then
        ::NPC_242_000::
        local sel = me:list(npc, '안녕하신가? 시간 좀 내주실 수 있겠나?', { '네, 무슨 일이세요?', '죄송합니다. 바빠서 이만.' })
        if sel == nil then
            return
        end
        if sel ~= 0 then
            return
        end

        ::NPC_242_001::
        local btn = me:dialog(npc, '나는 고구려 사람이라네.\n\n소문을 듣자 하니 이 섬에는 고구려에서는 볼 수 없는 신비한 효험을 지닌 약초들이 많이 자란다고 하더군.\n\n이곳의 토질을 조사하여 그 약초들을 재배할 수 있는 환경을 고구려에도 조성해 보자고 한다네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_242_000
        end

        ::NPC_242_002::
        btn = me:dialog(npc, '하지만 막상 와보니, 곳곳의 맹수, 괴수들 때문에 필요한 표본들을 모으기가 무척 힘들군.\n제대로 된 조사를 위해선 더욱 많은 양이 필요한데 큰일일세...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_242_001
        end

        sel = me:list(npc, '그래서 말인데.. 자네 혹시 도와줄 생각 없는가?\n내가 원하는 토양을 채집해 준다면, 은전으로 사례하겠네. 어떤가?', { '네, 해보죠.', '죄송합니다만, 힘들겠습니다.' })
        if sel == nil then
            return
        end
        if sel ~= 0 then
            return
        end

        quest = me:start_quest(QUEST_RED_CLAY)
        if quest == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest:progress(0)

        me:dialog(npc, '고맙네. 내가 필요로 하는 흙은 이렇게 생겼다네.\n\n고구려에서는 찾아볼 수 없는 것으로, 내가 적심토라 이름 붙혔지.\n냄세도 잘 기억해 두게나.\n\n괴수들이 있는 건처에서 적심토를 찾을 수 있을 것이네. 조심하게나.', false, true)
        return
    end

    if quest:completed() then
        local sel = me:list(npc, '자네로군, 나를 도와 다시 적심토를 채집하겠나?', { '좋습니다. 누구시더라?' })
        if sel == nil or sel ~= 0 then
            return
        end
        quest:resume()
        quest:progress(0)
        me:erase_achievement(22)
        me:dialog(npc, '괴수들이 있는 근처에서 적심토를 찾을 수 있을 것이네. 조심하게나.', false, true)
        return
    end

    local progress = quest:progress()
    if progress <= 0 then
        me:dialog(npc, '아직 적심토를 찾지 못한 모양이군. 괴수들이 있는 건처에서 구할수 있을걸세.', false, true)
        return
    end

    local money = progress * REWARD_PER_BAG
    local selected = me:list(npc, string.format('자네가 가져온 적심토 %d 꾸러미에 대한 보상으로 %d 전을 주겠네. 괜찮은가?', progress, money), { '좋습니다.', '지금은 싫습니다.' })
    if selected == nil or selected ~= 0 then
        return
    end

    selected = me:list(npc, '적심토 채집을 계속 하겠나?', { '네', '이제 그만할래요.' })
    if selected == nil then
        return
    end

    me:money(me:money() + money)
    quest:progress(0)
    me:erase_achievement(22)

    if selected == 0 then
        me:dialog(npc, '괴수들이 있는 근처에서 적심토를 찾을 수 있을 것이네. 조심하게나', false, true)
    else
        quest:complete()
        me:dialog(npc, '알겠네.. 그럼 언제든지 채집을 하고 싶거든 다시 오게.', false, true)
    end
end