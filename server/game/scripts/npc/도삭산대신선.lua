-- @note Trash: 1_도삭산\일반.txt "도삭산대신선". QUEST_DOJAECHUN step 1: hand in 신선의책 for 도삭산의서, warp to 900층주막, complete.

-- @brief   NPC Dosak Mountain great hermit: when QUEST_DOJAECHUN step is 1 and player has 신선의책, exchange for 도삭산의서 and warp to 900층주막.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (도삭산대신선).
function NPC_368(me, npc)
    local quest = me:quest(QUEST_DOJAECHUN)
    if quest == nil then
        return
    end
    if quest:completed() then
        return
    end
    if quest:step() ~= 1 then
        return
    end

    if not me:has_items("신선의책", 1) then
        return
    end

    ::NPC_368_0000::
    local button = me:dialog(npc, "자, 그럼 이 책의 마법을 시전하겠네. 신선의책에 담긴 힘을 봉인을 풀어 다시 원위치로 돌아가게 하는 것일세.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_368_0001::
    button = me:dialog(npc, "'현명한 옛 선인들의 지혜가 담겨 있는 신선의책이여, 신선의 이름으로 명하노니 도삭산의 모든 질서를 바로잡고 원래의 모습을 되찾도록 하여라!'", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_368_0000
    end

    ::NPC_368_0002::
    button = me:dialog(npc, "자네가 구해온 이 신선의책으로 인해서 이제 도삭산은 점점 질서를 찾아갈 것이네. 내 '도삭산의서'를 줄테니 소중히 간직하고 잘 쓰도록 하게나. 이 책은 도삭산의황금봉, 수정해골등과 함께 어딘가에 쓸 곳이 있을 것이네.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_368_0001
    end

    button = me:dialog(npc, "그럼 다음에 기회가 되면 또 보세..", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ["신선의책"] = 1 } },
        { ['item'] = { ["도삭산의서"] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "신선의책이 없지 않은가.", false, false)
        return
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 줄 수 없네.", false, false)
        return
    end

    local map = name2map("도삭산900층주막")
    if map then
        me:map(map, { 14, 14 })
    end

    me:push_achievement(47, "도삭산 900층 퀘스트 완료", 6, 25)
    quest:complete()
end
