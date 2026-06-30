-- npc: 만리장성잡화상09
function NPC_402(me, npc)
::NPC_402_000::
    local sel, btn = me:list(npc, "나에게 용무라도 있는가?", {"비약이란게 무엇입니까?", "그냥.. 시간이 남아서.."}, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 0 then
        local button = me:dialog(npc, "비약은.. 말 그대로 비밀의 약품이지. 소문에는 천지인의 패들을 서로 합치는데 비약이 필요하다고 하더군.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
    elseif sel == 1 then
        local messages = {
            "선비족 녀석들.. 고구려와 중국을 왕래하는 상인들을 습격한다고 하더라고. 나쁜놈들..",
            "고구려쪽 어딘가에 가면 선녀들이 사는 곳이 있다고 하던데.. 정말일까?",
            "만리장성에 있는 나 같은 잡화상들중에 천패, 지패, 인패를 마음대로 합칠 수 있는 사람이 있다는 소문이 있던데..",
            "자네, 국내성 왕궁엔 가보았는가? 요즘은 국내성의 왕궁에 일반인도 출입이 가능하다고 하던데..",
            "요즘 우리나라에 여러 괴물들이 여기저기서 나타나서 말이지.. 황제께서 직접 현상금을 거신 것 같더라고.",
        }
        local msg = messages[math.random(1, #messages)]
        local button = me:dialog(npc, msg, false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
    goto NPC_402_000
end
