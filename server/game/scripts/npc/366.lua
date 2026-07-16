-- npc: 일본통신사
local enum = require('lib.enum')
local NEED_ITEMS = {
    ["황도톨"] = 1, ["적도톨"] = 1, ["연녹도톨"] = 1, ["녹도톨"] = 1,
    ["청도톨"] = 1, ["회도톨"] = 1, ["갈도톨"] = 1, ["연갈도톨"] = 1,
    ["연자도톨"] = 1, ["자도톨"] = 1, ["연청도톨"] = 1, ["황금도톨"] = 1,
    ["황왕도톨"] = 1, ["적왕도톨"] = 1, ["연녹왕도톨"] = 1, ["녹왕도톨"] = 1,
    ["청왕도톨"] = 1, ["회왕도톨"] = 1, ["갈왕도톨"] = 1, ["연갈왕도톨"] = 1,
    ["연자왕도톨"] = 1, ["자왕도톨"] = 1, ["연청왕도톨"] = 1, ["황금왕도톨"] = 1,
}
local REWARD_NAME = "칠지도"

function NPC_366(me, npc)
    local d = me:dialog(npc, "안녕하십니까? 저는 일본과 부여를 오가며 외교를 담당하는 일본통신사 입니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "그런데 색깔별로 화려한 도톨이 있다는 소문을 들어서... 그것을 제가 가질 수 있다면, 칠지도를 본따 만든 검을 드리도록 하겠습니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end

    local sel, btn = me:list(npc, "도톨 24개를 가지고 있으신가요?", { "네. 모두 가져왔습니다.", "아닙니다... 다음에 다시..." }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 2 then
        me:dialog(npc, "준비가 되거든 다시 오세요...", false, false)
        return
    end
    if sel ~= 1 then
        return
    end

    local code = me:exchange(
        { ['item'] = NEED_ITEMS },
        { ['item'] = { [REWARD_NAME] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "24일간 모은 도톨 24종류를 가지고 오셔야 합니다.", false, false)
        return
    elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 칠지도를 드리지 못합니다. 도톨은 돌려드렸습니다.", false, false)
        return
    end

    d = me:dialog(npc, "도톨을 스물 네개개 모두 모으셨군요 !", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "여기 칠지도를 드리겠습니다. 이 검은 유리왕이 일왕에게 하사한 검을 본따 만든 것으로, 일정 시간이 지나면 자연히 사라지게 된답니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc, "그럼..", false, false)
end