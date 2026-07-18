-- npc: 해골NPC
local enum = require('lib.enum')
local TICKET_NAME = "교환권"
local TICKET_COUNT = 2
local REWARDS = {
    "해골죽장",
    "현철중검",
    "야월도",
    "영혼마령봉",
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

function NPC_345(me, npc)
    local sel, btn = me:list(npc, "교환권으로 무엇을 구입하시겠습니까?", {
        "해골죽장 (교환권 2개)",
        "현철중검 (교환권 2개)",
        "야월도 (교환권 2개)",
        "영혼마령봉 (교환권 2개)",
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel < 1 or sel > #REWARDS then
        return
    end
    do_exchange(me, npc, REWARDS[sel])
end