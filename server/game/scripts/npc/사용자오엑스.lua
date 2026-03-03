-- @note Trash: 2_이벤트\오엑스.txt "사용자오엑스". List: 물건 사기(callshop 117), 아무나오엑스입장(warp), 이벤트오엑스입장(gv oxquiz_open, warp). Shop via NPC_SELL_DIALOG if npc model has sell data.

---@brief   NPC 사용자오엑스: list (shop / 아무나오엑스 입장 / 이벤트오엑스 입장); warp by map name.
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
function NPC_309(me, npc)
    local sel, btn = me:list(npc, "무엇을 도와드릴까요?", {
        "물건 사기",
        "아무나오엑스입장",
        "이벤트오엑스입장",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if sel == 1 then
        local map = name2map("아무나오엑스시작")
        if map then
            me:map(map, math.random(13, 15), math.random(2, 4))
        else
            me:dialog(npc, "입장할 수 있는 장소가 없습니다.", false, false)
        end
        return
    end

    if sel == 2 then
        if gv("oxquiz_open") == 1 then
            local map = name2map("OX퀴즈장")
            if map then
                me:map(map, math.random(13, 15), math.random(2, 4))
            else
                me:dialog(npc, "입장할 수 있는 장소가 없습니다.", false, false)
            end
        else
            me:dialog(npc, "OX퀴즈 입장 시간이 지나 입장하실 수 없습니다.", false, false)
        end
    end
end
