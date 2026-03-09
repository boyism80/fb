
local TICKET_NAME = "교환권"

local REWARDS = {
    { count = 25,   name = "인어반지",   label = "인어반지 (25개)" },
    { count = 50,   name = "진주반지",   label = "진주반지 (50개)" },
    { count = 100,  name = "주홍투구",   label = "주홍투구 (100개)" },
    { count = 1250, name = "정화의방패", label = "정화의방패 (1250개)" },
    { count = 1500, name = "용왕의반지", label = "용왕의반지 (1500개)" },
    { count = 2000, name = "용왕의투구", label = "용왕의투구 (2000개)" },
}

local function do_exchange(me, npc, need_count, reward_name)
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = need_count } },
        { ['item'] = { [reward_name] = 1 } }
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

function NPC_350(me, npc)
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
    do_exchange(me, npc, r.count, r.name)
end
