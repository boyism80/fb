-- npc: 만리장성잡화상04

return {
    on_click = function(me, npc)
        local messages = {
            "흉노족 두령은 생긴건 무섭게 생겼지만 알고보면 말이 잘 통하는 똑똑한 사람이라던데.. 역시 사람은 겉만 보곤 알 수 없는건가?",
            "대방성 대장장이 녀석이 검정 깃발을 혼자 가지고 있다는 소문이 있어.",
            "흉노족은 말이야. 장로가 있고 두령이 있더라고. 그럼 도대체 누가 대장인거야?",
            "선비족 녀석들.. 고구려와 중국을 왕래하는 상인들을 습격한다고 하더라고. 나쁜놈들..",
            "장안성은 세상에서 제일 크고 멋진 성이 아닐까?",
        }
        local msg = messages[math.random(1, #messages)]
        local button = me:dialog(npc, msg, { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
