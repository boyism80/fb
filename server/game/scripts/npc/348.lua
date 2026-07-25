-- npc: 백호왕NPC
local enum = require('lib.enum')
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

local function do_exchange(me, npc, need_count, reward_name, reward_count)
    local code = me:exchange(
        { ['item'] = { [TICKET_NAME] = need_count } },
        { ['item'] = { [reward_name] = reward_count } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "교환권이 부족하시네요.", { prev = false, next = false })
        return false
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. name_with(reward_name, '을', '를') .. " 드리지 못합니다. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
        return false
    end
    me:dialog(npc, "여기 있습니다..", { prev = false, next = false })
    return true
end

return {
    ON_CLICK = function(me, npc)
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
        do_exchange(me, npc, r.count, r.name, r.count)
    end
}
