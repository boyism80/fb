-- @note Trash: 2_이벤트\폭피.txt "폭노폭노". list: 물건 사기(callshop 0), 물건 팔기(callshop 1), 귀환(nation-based warp). FB: use NPC_SELL_DIALOG/NPC_BUY_DIALOG (npc:model() sell/buy); 귀환 warps to 국내성 (nation-based map array omitted).

---@brief   NPC 폭노폭노: Bomb-avoid event NPC; list (buy/sell/return). Uses same shop pattern as 국내포목 (NPC_SELL_DIALOG, NPC_BUY_DIALOG). Return warps to 국내성.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_487(me, npc)
    ::NPC_487_000::
    local sel, btn = me:list(npc, "안녕하세요. 어떻게 오셨나요?", {
        "물건 사기",
        "물건 팔기",
        "귀환",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.QUIT then
            return
        end
        goto NPC_487_000
    end
    if sel == 1 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT.QUIT then
            return
        end
        goto NPC_487_000
    end
    if sel == 2 then
        local map = name2map("국내성")
        if map then
            me:map(map, math.random(3, 8), math.random(5, 9))
        else
            me:dialog(npc, "이동할 수 없습니다.", false, false)
        end
        return
    end
end
