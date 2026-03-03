-- @note Trash: 2_이벤트\부여장터.txt "백호왕NPC". Exchange 교환권 for 건괘(2), 곤괘(2), 감괘(3), 진괘(3), 리괘(4), 선괘(4), 태괘(4), 간괘(16), 용의비늘(10).

local TICKET_NAME = "교환권"

local REWARDS = {
    { count = 2,  name = "건괘",  label = "건괘 (2개)" },
    { count = 2,  name = "곤괘",  label = "곤괘 (2개)" },
    { count = 3,  name = "감괘",  label = "감괘 (3개)" },
    { count = 3,  name = "진괘",  label = "진괘 (3개)" },
    { count = 4,  name = "리괘",  label = "리괘 (4개)" },
    { count = 4,  name = "선괘",  label = "선괘 (4개)" },
    { count = 4,  name = "태괘",  label = "태괘 (4개)" },
    { count = 16, name = "간괘",  label = "간괘 (16개)" },
    { count = 10, name = "용의비늘", label = "용의비늘 (10개)" },
}

---@brief Try to exchange tickets for one reward; show dialog on success or failure.
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
---@param[in] need_count  Number of 교환권 required.
---@param[in] reward_name Item name to give.
---@param[in] reward_count Number of reward item to give.
---@return boolean True if exchange succeeded.
local function do_exchange(me, npc, need_count, reward_name, reward_count)
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = need_count } },
        { ['item'] = { [reward_name] = reward_count } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "교환권이 부족하시네요.", false, false)
        return false
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. name_with(reward_name, '을', '를') .. " 드리지 못합니다. 자리 좀 비우고 다시 오세요.", false, false)
        return false
    end
    me:dialog(npc, "여기 있습니다..", false, false)
    return true
end

---@brief NPC 백호왕NPC: exchange 교환권 for one of nine items (different counts).
---@param[in] me  The character.
---@param[in] npc The NPC entity.
function NPC_348(me, npc)
    local labels = {}
    for i = 1, #REWARDS do
        labels[i] = REWARDS[i].label
    end
    local sel, btn = me:list(npc, "교환권으로 무엇을 구입하시겠습니까?", labels, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel < 0 or sel >= #REWARDS then
        return
    end
    local r = REWARDS[sel + 1]
    do_exchange(me, npc, r.count, r.name, r.count)
end
