-- @note Trash: 1_도삭산\일반.txt "도나영". $dojaechun==0: 2 dialogs. $dojaechun==1: thanks + 1000층 hint; if has 수정해골+도삭산황금봉+도삭산의서 then list→warp to 1000층상점, broadcast, achievement.

-- @brief   NPC Do Na-young: when QUEST_DOJAECHUN not at step 1/completed shows family dialog; when step 1 or completed, thanks and offers warp to 1000층상점 if player has the three items.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (도나영).
function NPC_370(me, npc)
    local quest = me:quest(QUEST_DOJAECHUN)
    local dojaechun_one = (quest and (quest:step() == 1 or quest:completed()))

    if not dojaechun_one then
        ::NPC_370_0000::
        local button = me:dialog(npc, "큰아버지께서 요즘 건강이 안 좋으셔서 그 집 가족들이 모두 걱정하고 있어요.. 어서 건강해지셔야 할텐데..", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        button = me:dialog(npc, "저희 가족들은 도삭산 여기저기에 흩어져서 살고 있답니다. 가끔 모두 모여서 재미있는 이야기들을 하기도 하지요.", true, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_370_0000
        end
        return
    end

    ::NPC_370_0001::
    local button = me:dialog(npc, string.format("앗! %s님이시군요! 저희 가족들 모두를 대신해서 제가 감사드립니다.", me:name()), false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_370_0002::
    button = me:dialog(npc, "보답이라도 해드려야 할 것 같은데.. 도삭산에는 1000층 상점이 숨겨져 있는데요, 제가 거기로 가는 길을 알고 있거든요.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_370_0001
    end

    ::NPC_370_0003::
    button = me:dialog(npc, "여길 가려면, 수정해골, 도삭산황금봉, 도삭산의서.. 세가지가 필요하답니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_370_0002
    end

    local required = {
        ["수정해골"] = 1,
        ["도삭산황금봉"] = 1,
        ["도삭산의서"] = 1,
    }
    if not me:has_items("수정해골", 1) or not me:has_items("도삭산황금봉", 1) or not me:has_items("도삭산의서", 1) then
        return
    end

    ::NPC_370_0004::
    button = me:dialog(npc, "앗, 이미 가지고 계시군요? 그럼 비밀의 1000층상점으로 보내드리겠습니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_370_0005::
    button = me:dialog(npc, "주의하실 점은, 단 한번만 가실 수 있다는 것과, 여기서 파는 물건들은 굉장히 좋은 것들이지만, 그만큼 가격이 비싸다는 것도 기억해두세요.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_370_0004
    end

    local sel, btn = me:list(npc, "지금 가시겠어요?", { "네, 가겠습니다.", "잠시후에.." }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel ~= 0 then
        return
    end

    if not me:rmitem(required, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, "필요한 물건이 없어 보이네요.", false, false)
        return
    end

    local map = name2map("도삭산1000층상점")
    if map then
        me:map(map, { 7, 13 })
    end

    broadcast(string.format("<<%s>> 님이 도삭산 1000층에 도착하셨습니다!!", me:name()), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
    me:push_achievement(44, "도삭산 1000층 도착!", 6, 25)
end
