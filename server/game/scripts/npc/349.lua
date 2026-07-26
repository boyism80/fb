-- npc: 어전NPC
local enum = require('lib.enum')
local TICKET_NAME = "교환권"

local REWARDS = {
    { count = 100,  name = "어전의칼",     label = "어전의칼 (100개)" },
    { count = 50,   name = "도깨비가죽",   label = "도깨비가죽 (50개)" },
    { count = 25,   name = "하선녀의실타래", label = "하선녀의실타래 (25개)" },
    { count = 50,   name = "불의수정",     label = "불의수정 (50개)" },
    { count = 1500, name = "유성지의보패", label = "유성지의보패 (1500개)" },
    { count = 2000, name = "해골왕의뼈",   label = "해골왕의뼈 (2000개)" },
    { count = 20,   name = "이가닌자의독",  label = "이가닌자의독 (20개)" },
    { count = 20,   name = "이가닌자의보패", label = "이가닌자의보패 (20개)" },
    { count = 20,   name = "흑룡철심",    label = "흑룡철심 (20개)" },
    { count = 20,   name = "수리검",      label = "수리검 (20개)" },
    { count = 20,   name = "검조각",      label = "검조각 (20개)" },
}

local function do_exchange(me, npc, need_count, reward_name)
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = need_count } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "교환권이 부족하시네요.", { prev = false, next = false })
        return false
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. name_with(reward_name, '을', '를') .. " 드리지 못합니다. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
        return false
    end
    me:dialog(npc, "여기 있습니다..", { prev = false, next = false })
    return true
end

return {
    on_click = function(me, npc)
        local labels = {}
        for i = 1, #REWARDS do
            labels[i] = REWARDS[i].label
        end
        local sel, btn = me:list(npc, "교환권으로 무엇을 구입하시겠습니까?", labels, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel < 1 or sel > #REWARDS then
            return
        end
        local r = REWARDS[sel]
        do_exchange(me, npc, r.count, r.name)
    end
}
