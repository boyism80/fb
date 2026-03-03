-- @note Trash: 0_금화관련\일반.txt "금화상인". Gold-coin exchange: list menu; each branch calls external func (기간제무기대여, 고급아이템구입, etc.) — placeholder per branch.

-- @brief   NPC Gold-coin merchant: intro and menu (time-limited weapons, premium items, boxes, exp, convenience, refund). Branches not implemented.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (금화상인).
function NPC_386(me, npc)
    local button = me:dialog(npc, "안녕하세요? 금화를 특별한 아이템으로 교환해 드리는 금화교환 도우미라고 해요.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "무엇을 교환 해 드릴까요?  (＊시간제 아이템은 빽섭등으로 인해 시간을 날리더라도 보상되지 않습니다.)", {
        "기간제 무기",
        "고급 아이템",
        "상자/주머니 관련",
        "경험치 관련",
        "편의 아이템",
        "(기간한정) 금화 아이템 환불"
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
