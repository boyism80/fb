-- npc: 흉가NPC
local enum = require('lib.enum')
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

local function do_exchange(me, npc, need_count, reward_name)
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = need_count } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "교환권이 부족하시네요.", false, false)
        return false
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. name_with(reward_name, '을', '를') .. " 드리지 못합니다.", false, false)
        return false
    end
    me:dialog(npc, "여기 있습니다..", false, false)
    return true
end

function NPC_347(me, npc)
    local labels = {}
    for i = 1, #REWARDS do
        labels[i] = REWARDS[i].label
    end
    local sel, btn = me:list(npc, "교환권 1개로 무엇을 구입하시겠습니까?", labels, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel < 1 or sel >= #REWARDS then
        return
    end
    local r = REWARDS[sel]
    do_exchange(me, npc, r.count, r.name)
end