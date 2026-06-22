-- npc: 설원세공사3
local enum = require('lib.enum')
local MATERIAL_NAME = "눈꽃얼음"
local MATERIAL_COUNT = 10
local REWARD_NAME = "눈꽃반지"
local SUCCESS_PERCENT = 30

local function do_craft(me, npc)
    local cost = { ['item'] = { [MATERIAL_NAME] = MATERIAL_COUNT } }
    local reward = nil
    if math.random(1, 100) <= SUCCESS_PERCENT then
        reward = { ['item'] = { [REWARD_NAME] = 1 } }
    end
    local code = me:exchange(cost, reward)
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "눈꽃얼음이 없으신데요?", false, false)
        return false
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. name_with(REWARD_NAME, '을', '를') .. " 드리지 못합니다.", false, false)
        return true
    end
    if reward == nil then
        me:dialog(npc, "이런... 그만 실패하고 말았군요. 다음 기회를 기약하셔야 할 듯 싶네요...", false, false)
    else
        me:dialog(npc, "오... 이것이 " .. REWARD_NAME .. "로군요. 이렇게 아름답게 생긴 것은 처음입니다. 잘 사용하시길 바라겠습니다.", false, false)
    end
    return true
end

function NPC_436(me, npc)
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
    d = me:dialog(npc, "하지만 한번 가공하게 되면, 단단히 얼어붙어 그 무엇보다 견고한 결과물이 나오기 마련입니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "눈꽃얼음을 이용해 [" .. REWARD_NAME .. "]을 만들어 드리겠습니다. 하지만 성공은 장담하지 못하는데... 괜찮으시겠습니까?", false, true)
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

    d = me:dialog(npc, "자... 그럼 한번 만들어 보겠습니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "읏차라 읏차 읏차차, 요렇게 저렇게 숑숑~", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "이렇게 잘 해서 이리저리 얍얍~", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "어디 한번 잘 되었나 볼까?!", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end

    do_craft(me, npc)
end
