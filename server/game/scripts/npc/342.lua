-- npc: 게이바주인
local enum = require('lib.enum')

function NPC_342(me, npc)
    local exchanges = {
        { need = 30, item = "트렁크", msg = "좋아. 여기 트렁크를 주지. 하지만 별로 남자다운 의상은 아니군!" },
        { need = 50, item = "비키니", msg = "좋아. 여기 비키니를 주지. 비키니야 말로 진정 남자다운 옷이란다!" },
        { need = 80, item = "채찍", msg = "좋아. 여기 채찍을 주지. 정말 찰진 채찍이라 이것을 사용하면 기분이 좋아질거야!" },
        { need = 10, item = "완전한사랑", msg = "아쉽군! 이제는 너의 그 부드러움을 못느끼게 되었어. 우리의 완전한 사랑이 그 결실을 맺었으니, 이걸 사용하도록 해!" },
    }
    local insufficient_msg = "아직 너의 사랑이 부족한것 같은데?"

    ::NPC_342_000::
    local sel, btn = me:list(npc, "들어올땐 마음대로지만 나갈땐 아니란다?", {
        "트렁크 교환 (사랑의증표 30개)",
        "비키니 교환 (사랑의증표 50개)",
        "채찍 교환 (사랑의증표 80개)",
        "사랑을 10개 모아서 나가고싶다.",
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel < 1 or sel > 4 then
        return
    end

    local ex = exchanges[sel]
    local code = me:exchange(
        { ['item'] = { ["사랑의증표"] = ex.need } },
        { ['item'] = { [ex.item] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, insufficient_msg, { prev = false, next = false })
        goto NPC_342_000
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 받을 수 없네요. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
        goto NPC_342_000
    end

    me:dialog(npc, ex.msg, { prev = false, next = false })
    goto NPC_342_000
end