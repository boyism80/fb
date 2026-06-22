-- npc: 설원조합사3
local server = require('lib.server')
local MATERIAL_NAME = "눈꽃얼음"
local MATERIAL_COUNT = 50
local REWARD_NAME = "활력의귀걸이"
local SUCCESS_PERCENT = 20

local function do_craft(me, npc)
    local cost = { [MATERIAL_NAME] = MATERIAL_COUNT }
    local reward = nil
    if math.random(1, 100) <= SUCCESS_PERCENT then
        reward = { ['item'] = { [REWARD_NAME] = 1 } }
    end
    local code = me:exchange({ ['item'] = cost }, reward)
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "눈꽃얼음이 없으신데요?", false, false)
        return false
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. REWARD_NAME .. "을 드리지 못합니다.", false, false)
        return true
    end
    if reward == nil then
        me:dialog(npc, "이런... 그만 실패하고 말았군요. 다음 기회를 기약하셔야 할 듯 싶네요...", false, false)
    else
        me:dialog(npc, "오... " .. REWARD_NAME .. "을 만드는데에 성공했습니다!! 잘 사용하시길 바라겠습니다.", false, false)
    end
    return true
end

function NPC_440(me, npc)
    local sel, btn = me:list(npc, "저는 눈꽃얼음을 이용해 [" .. REWARD_NAME .. "]을 만들어 드립니다. ", { "만들어 주세요.", "아니요. 다음에..." }, false)
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

    local d = me:dialog(npc, "안녕하십니까? 눈꽃얼음은 그 특성상 그 재료가 매우 부서지기 쉽기 때문에 가공이 매우 힘들답니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "눈꽃얼음을 50개 가져오시면, 제 나름대로 심혈을 기울여 " .. REWARD_NAME .. "을 만들어 볼 수 있습니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "눈꽃얼음 50개를 가져오시면 [" .. REWARD_NAME .. "]을 만들어 드릴 수 있습니다. 하지만 성공은 장담하지 못하는데... 괜찮으시겠습니까?", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end

    sel, btn = me:list(npc, "어떻게... 한번 시도를 해 볼까요?", { "네. 각오했습니다.", "아니요. 역시 무섭네요..." }, false)
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

    d = me:dialog(npc, "자... 그럼 눈꽃얼음을 이용해 한번 " .. REWARD_NAME .. "을 만들어 보도록 하겠습니다.", false, true)
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

    do_craft(me, npc)
end
