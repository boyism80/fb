-- npc: 도깨비NPC
local enum = require('lib.enum')
local TICKET_NAME = "교환권"
local TICKET_COUNT = 10
local REWARDS = {
    "가시철단도",
    "환두대도",
    "대마령봉",
    "삼촉현창",
}

local function do_exchange(me, npc, reward_name)
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = TICKET_COUNT } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "교환권이 부족하시네요.", { prev = false, next = false })
        return false
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. reward_name .. "을 드리지 못합니다.", { prev = false, next = false })
        return false
    end
    me:dialog(npc, "여기 있습니다..", { prev = false, next = false })
    return true
end

return {
    ON_CLICK = function(me, npc)
        local sel, btn = me:list(npc, "교환권으로 무엇을 구입하시겠습니까?", {
            "가시철단도 (교환권 10개)",
            "환두대도 (교환권 10개)",
            "대마령봉 (교환권 10개)",
            "삼촉현창 (교환권 10개)",
        }, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel < 1 or sel > #REWARDS then
            return
        end
        do_exchange(me, npc, REWARDS[sel])
    end
}
