-- npc: 만리장성잡화상10

return {
    ON_CLICK = function(me, npc)
        local messages = {
            "고구려엔 왈숙네라는 주모가 있다면서? 어떤 주모이길래 중국에 까지 소문이 나는걸까?",
            "신부여성 서쪽엔 세시마을이라는 곳이 있더군. 한번 가본적이 있는데.. 굉장히 특이한 마을인 것 같던데 말야.",
            "암흑왕인가 하는 굉장히 무서운 괴물이 어디에 살고 있다던데.. 어떻게 생긴걸까?",
            "중국경비병과 고구려경비병이 싸우면 누가이길까?",
            "요즘 우리나라에 여러 괴물들이 여기저기서 나타나서 말이지.. 황제께서 직접 현상금을 거신 것 같더라고.",
        }
        local msg = messages[math.random(1, #messages)]
        local button = me:dialog(npc, msg, { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
