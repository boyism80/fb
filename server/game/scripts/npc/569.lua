-- npc: 수정노인
local enum = require('lib.enum')
local MATERIAL = "수정"
local RING_NAMES = { "수정반지1", "수정반지2", "수정반지3", "수정반지4", "수정반지5" }

function NPC_569(me, npc)
    if not me:has_items(MATERIAL, 1) then
        me:dialog(npc, "나는 " .. MATERIAL .. "으로 세공을 한다네...", { prev = false, next = false })
        return
    end

    ::NPC_569_0001::
    local d = me:dialog(npc, "오... 자네 " .. MATERIAL .. "을 가지고 있군.. 어디 몇 개나 가지고 있나 볼까.", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_569_0002::
    local sel, btn = me:list(npc, "나는 " .. MATERIAL .. "으로 " .. MATERIAL .. "반지를 만들 수 있다네. 어떻게 하겠는가?", {
        "수정반지1",
        "수정반지2",
        "수정반지3",
        "수정반지4",
        "수정반지5",
    }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_569_0001
    end
    if sel == nil then
        return
    end
    local need_count = sel
    local reward_name = RING_NAMES[sel]

    ::NPC_569_0003::
    d = me:dialog(npc, "그래... " .. reward_name .. "을 만들고 싶다는 말이지?", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_569_0004::
    local confirm, confirm_btn = me:list(npc, "내게 " .. MATERIAL .. "을 " .. need_count .. "개 준다면 " .. reward_name .. "를 만들어주지... 어떻게 하겠는가?", { "만들어주세요.", "아니요...괜찮습니다." }, { prev = true })
    if confirm_btn == DIALOG_RESULT.QUIT then
        return
    end
    if confirm_btn == DIALOG_RESULT.PREV then
        goto NPC_569_0003
    end
    if confirm ~= 1 then
        me:dialog(npc, "그래, 생각이 바뀐다면 다시 오게나. 허허...", { prev = false, next = false })
        return
    end

    local code = me:exchange(
        { ['item'] = { [MATERIAL] = need_count } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "음... 자네는 가진 " .. MATERIAL .. "이 부족한 것 같군...", { prev = false, next = false })
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. reward_name .. "을 드리지 못합니다.", { prev = false, next = false })
        return
    end
    me:dialog(npc, reward_name .. " 받았습니다. 잘 쓰게나.", { prev = false, next = false })
end