-- @note Trash: 2_이벤트\부여장터.txt "흉가NPC". Exchange 교환권 1 (or 10 for 황금호박) for amber items (호박 = amber).
-- List: 녹호박, 갈호박, 자호박, 연갈호박, 연녹호박, 연자호박, 연청호박, 적호박, 청호박, 황금호박(10), 황호박, 회호박.

local TICKET_NAME = "교환권"

local REWARDS = {
    { count = 1,  name = "녹호박",  label = "녹호박" },
    { count = 1,  name = "갈호박",  label = "갈호박" },
    { count = 1,  name = "자호박",  label = "자호박" },
    { count = 1,  name = "연갈호박", label = "연갈호박" },
    { count = 1,  name = "연녹호박", label = "연녹호박" },
    { count = 1,  name = "연자호박", label = "연자호박" },
    { count = 1,  name = "연청호박", label = "연청호박" },
    { count = 1,  name = "적호박",  label = "적호박" },
    { count = 1,  name = "청호박",  label = "청호박" },
    { count = 10, name = "황금호박", label = "황금호박 (10개)" },
    { count = 1,  name = "황호박",  label = "황호박" },
    { count = 1,  name = "회호박",  label = "회호박" },
}

---@brief Try to exchange tickets for one reward; show dialog on success or failure.
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
---@param[in] need_count  Number of 교환권 required.
---@param[in] reward_name Item name to give.
---@return boolean True if exchange succeeded.
local function do_exchange(me, npc, need_count, reward_name)
    if not me:has_items(TICKET_NAME, need_count) then
        me:dialog(npc, "교환권이 부족하시네요.", false, false)
        return false
    end
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = need_count } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "교환권이 부족하시네요.", false, false)
        return false
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. reward_name .. "을(를) 드리지 못합니다.", false, false)
        return false
    end
    me:dialog(npc, "여기 있습니다..", false, false)
    return true
end

---@brief NPC 흉가NPC: exchange 교환권 1 (or 10 for 황금호박) for one of 12 amber items.
---@param[in] me  The character.
---@param[in] npc The NPC entity.
function NPC_347(me, npc)
    local labels = {}
    for i = 1, #REWARDS do
        labels[i] = REWARDS[i].label
    end
    local sel, btn = me:list(npc, "교환권 1개로 무엇을 구입하시겠습니까?", labels, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel < 0 or sel >= #REWARDS then
        return
    end
    local r = REWARDS[sel + 1]
    do_exchange(me, npc, r.count, r.name)
end
