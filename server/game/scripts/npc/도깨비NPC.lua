-- @note Trash: 2_이벤트\부여장터.txt "도깨비NPC". Exchange 교환권 10 for one of 가시철단도, 환두대도, 대마령봉, 삼촉현창.

local TICKET_NAME = "교환권"
local TICKET_COUNT = 10
local REWARDS = {
    "가시철단도",
    "환두대도",
    "대마령봉",
    "삼촉현창",
}

---@brief Try to exchange TICKET_COUNT 교환권 for one reward item; show dialog on success or failure.
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
---@param[in] reward_name  Item name to give.
---@return boolean True if exchange succeeded.
local function do_exchange(me, npc, reward_name)
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = TICKET_COUNT } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "교환권이 부족하시네요.", false, false)
        return false
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. reward_name .. "을 드리지 못합니다.", false, false)
        return false
    end
    me:dialog(npc, "여기 있습니다..", false, false)
    return true
end

---@brief NPC 도깨비NPC: exchange 교환권 10 for one of four weapons.
---@param[in] me  The character.
---@param[in] npc The NPC entity.
function NPC_346(me, npc)
    local sel, btn = me:list(npc, "교환권으로 무엇을 구입하시겠습니까?", {
        "가시철단도 (교환권 10개)",
        "환두대도 (교환권 10개)",
        "대마령봉 (교환권 10개)",
        "삼촉현창 (교환권 10개)",
    }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel < 0 or sel >= #REWARDS then
        return
    end
    do_exchange(me, npc, REWARDS[sel + 1])
end
