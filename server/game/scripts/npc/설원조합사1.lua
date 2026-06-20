local server = require('lib.server')
local REWARD_NAME = "눈꽃얼음"
local NEED = { ["얼음결정"] = 50, ["설인의뼈"] = 1 }

local function do_craft(me, npc)
    local code = me:exchange(
        { ['item'] = NEED },
        { ['item'] = { [REWARD_NAME] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "재료가 부족하시군요.", false, false)
        return false
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 눈꽃얼음을 드리지 못합니다.", false, false)
        return false
    end
    return true
end

function NPC_438(me, npc)
    local sel, btn = me:list(npc, "저는 얼음결정과 설인의뼈를 이용해 [" .. REWARD_NAME .. "]을 만들어 드립니다. ", { "만들어 주세요.", "아니요. 다음에..." }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 1 then
        me:dialog(npc, "잘 생각 하셨습니다.", false, false)
        return
    end
    if sel ~= 0 then
        return
    end

    local d = me:dialog(npc, "안녕하십니까? 얼음결정은 그 특성상 그 재료가 매우 부서지기 쉽기 때문에 가공이 매우 힘들답니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "그렇게 설인의뼈 한개와, 얼음결정 50개를 깎아 만들게 되면, 눈꽃얼음 이라는 특별한 재료가 만들어지게 된답니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "설인의뼈 1개와 얼음결정 50개를 이용해 [" .. REWARD_NAME .. "]을 만들어 드릴 수 있습니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end

    sel, btn = me:list(npc, "만들어 드릴까요?", { "네. 만들어 주세요.", "아니요. 나중에 오겠습니다." }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 1 then
        me:dialog(npc, "잘 생각 하셨습니다.", false, false)
        return
    end
    if sel ~= 0 then
        return
    end

    d = me:dialog(npc, "자... 그럼 설인의뼈로 한번 얼음결정을 깎아 보도록 하겠습니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "아주 신중하고... 또 신중하게.......", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "심혈을 기울여서......", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "사각 사각.....", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end

    if not do_craft(me, npc) then
        return
    end
    me:dialog(npc, REWARD_NAME .. "을 만들어 드렸으니, 잘 사용하시길 바라겠습니다.", false, false)
end
