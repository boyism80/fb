-- npc: 남북무한도우미
function NPC_499(me, npc)
::NPC_499_MENU::
    local sel, list_btn = me:list(npc, "안녕하세요. 어떻게 오셨나요?", {
        "설명듣기",
        "남북무한대전 참가",
        "남북무한대전 현황",
        "상품수령",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 1 then
        local btn = me:dialog(npc, "안녕하세요? " .. me:name() .. "님. 남북무한대전에 오신 것을 환영합니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "남북무한대전은, 참여한 유저들이 남쪽과 북쪽팀으로 나뉘어, 적의 대장을 잡으면 승리하는 게임입니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "진영은 랜덤하게 정해지게 되고, 한번 정해진 진영은 다시 변경할 수 없습니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "1시간동안 진행되는 남북무한대전은, 중간에 한 쪽 진영의 대장이 죽게 되면 그 즉시 종료됩니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "또한 공지사항을 통해 사전에 알려드린 시간에만 참여하실 수 있습니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "만약 아이템창을 꽉 채우고 참여하실 경우 상품을 받지 못하실 수도 있습니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "또한, 아이템 창이 모자른 경우, 대전중 아이템을 바닥에 떨어뜨릴 수도 있으니, 반드시 사전에 아이템창을 넉넉히 비우고 참여하시기 바랍니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "[" .. me:name() .. "]님에게 행운이 있으시기를 바랍니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        goto NPC_499_MENU
    end

    if sel == 2 then
        me:dialog(npc, "남북무한대전 참가 신청은 공지된 일정에만 가능합니다. 현재는 진행 시간이 아니거나 준비중입니다.", false, true)
        goto NPC_499_MENU
    end

    if sel == 3 then
        me:dialog(npc, "남북무한대전 현황 조회는 현재 준비중입니다.", false, true)
        goto NPC_499_MENU
    end

    if sel == 4 then
        local ns_start = property("ns_start")
        if ns_start == 2 then
            local winner = property("ns_winner_team")
            me:dialog(npc, "승리팀 상품 수령은 대전 종료 후 해당 기능이 연동되면 이용하실 수 있습니다.", false, true)
        else
            me:dialog(npc, "수령할 상품이 없습니다. 남북무한대전에 참가하고 승리한 뒤 이용해 주세요.", false, true)
        end
        goto NPC_499_MENU
    end
end