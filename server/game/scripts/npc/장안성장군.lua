function NPC_231(me, npc)
    local quest = me:quest(QUEST_HWANGBIYEON)

    if quest == nil then
        local btn = me:dialog(npc, '황궁을 견학하러 왔는가? 허허, 미안하네만 지금은 보시다시피 좀 어수선하다네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, lb = me:list(npc, '자네 혹시 황비연이라는 자를 아는가?', { '잘 모르겠는데요. 그게 누굽니까?', '전 아무것도 모릅니다. 그럼 이만...' }, true)
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == 1 then
            me:dialog(npc, '허허.. 싱겁기는..', false, true)
            return
        end
        if sel ~= 0 then
            return
        end
        btn = me:dialog(npc, '음... 황비연이라는 자는 요즘 기승을 부리고 있는 신출귀몰한 도둑이지. 자칭 의적으로, 검은 머리띠를 두르고 나타나 탐관오리들의 재물을 훔쳐 가난한 사람들에게 나눠 준다는군.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        sel, lb = me:list(npc, '백성들도 황비연에게 도움을 많이 받고 있는 모양이야. 그에 대해 물으면 다들 모른다고 잡아떼더군. 허허, 이거야 원...', { '의적이군요? 그럼 슬쩍 눈감아줘도 되지 않을까요?', '고생이 많으시네요. 저는 바쁜 일이 있어서 이만..' }, true)
        if lb == DIALOG_RESULT.QUIT or sel ~= 0 then
            return
        end
        ::NPC_231_COS001::
        btn = me:dialog(npc, '...실은 나도 그렇게 하고 싶지만, 상부에서 그를 잡아들이라고 난리라서 어쩔 수 없다네. 현상금도 크게 걸려 있지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '사실 탐관오리들이 부정하게 모아온 재물이긴 하지만, 어찌 되었건 간에 도둑질은 나쁜 일이지 않은가.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_231_COS001
        end
        sel, lb = me:list(npc, '그래서 지금도 황비연을 잡으려고 수색중인데, 정말 신출귀몰하더군. 도저히 잡을 수가 없어...', { '저도 협조하겠습니다. 황비연을 만나보고 싶군요.', '솔직히 잡히지 말았으면 좋겠군요. 그럼 전 이만..' }, true)
        if lb == DIALOG_RESULT.QUIT or sel ~= 0 then
            return
        end
        ::NPC_231_COS002::
        btn = me:dialog(npc, '음! 정말 그래 주겠는가? 고맙네. 자네는 황궁 사람이 아니니 어쩌면 황비연에게 쉽게 접글할 수 있을지도 모르지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '황비연이 마지막으로 나타난 장소는 상해일세. 그 부근에 숨어 있을 것 같으니 잘 살펴 보게나. 그럼 수고해주게.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_231_COS002
        end
        quest = me:start_quest(QUEST_HWANGBIYEON)
        if quest == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest:step(1)
        me:push_achievement(39, '황비연을 만나러 상해로 가자.', 7, 2)
        return
    end

    if quest:completed() then
        me:dialog(npc, '안녕하신가? 자네 덕분에 난 진급을하여 편한안 삶을 살고 있지. 정말 고맙네.', true, true)
    end

    if quest:step() >= 1 and quest:step() <= 3 then
        me:dialog(npc, '한번 상해로 가보십시요. 황비연이 마지막으로 나타난 장소가 상해입니다.', true, true)
        return
    end

    if quest:step() == 4 then
        local sel, lb = me:list(npc, '황비연은 잡아왔는가?', { '그는 죽었고, 그의 머리띠를 가져왔습니다.', '아니오, 아직...' }, true)
        if lb == DIALOG_RESULT.QUIT or sel ~= 0 then
            return
        end
        if not me:has_items('황비연의머리띠', 1) then
            me:dialog(npc, '머리띠가 없는것 같은데요?', false, true)
            return
        end
        ::NPC_231_COS003::
        local btn = me:dialog(npc, '오, 머리띠라... 어디 보세.\n\n음! 과연... 이것은 그가 항상 머리에 두르고 있다는 그 머리띠가 분명하군!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '그가 정말로 죽었는지 어떤지는 자네만이 알고 있겠지만... 허허허.\n\n뭐, 이 머리띠면 상부에 황비연이 죽었다고 보고할 수 있겠지. 아무튼 수고했네. 여기 현상금 10만전을 가져가게.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_231_COS003
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = { ['황비연의머리띠'] = 1 } },
            { ['money'] = 100000 }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '황비연의머리띠를 가져오세요.', false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '금전을 받을 여유가 없군요.', false, false)
            return
        end
        quest:complete()
        me:push_achievement(39, '의적 황비연 임무를 완벽히 수행하였다.', 7, 2)
        return
    end
end