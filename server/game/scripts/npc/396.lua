-- npc: 만리장성잡화상03
function NPC_396(me, npc)
    local messages = {
        "선비족 녀석들.. 고구려와 중국을 왕래하는 상인들을 습격한다고 하더라고. 나쁜놈들..",
        "고구려쪽 어딘가에 가면 선녀들이 사는 곳이 있다고 하던데.. 정말일까?",
        "만리장성에 있는 나 같은 잡화상들중에 천패, 지패, 인패를 마음대로 합칠 수 있는 사람이 있다는 소문이 있던데..",
        "자네, 국내성 왕궁엔 가보았는가? 요즘은 국내성의 왕궁에 일반인도 출입이 가능하다고 하던데..",
        "장안성은 세상에서 제일 크고 멋진 성이 아닐까?",
    }
    local msg = messages[math.random(1, #messages)]
    local button = me:dialog(npc, msg, { prev = false, next = false })
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
