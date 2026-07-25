local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '안녕하세요. 어떻게 오셨나요?',
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
                { '물건 팔기', function(me, ch)
                    return npc.show_buy_menu(me, ch)
                end },
                { '끈옷판매', function(me, ch)
                    return me:dialog(ch, '특별 이벤트 기간에만 가능합니다.', { prev = false, next = true })
                end },
                { '산타클로스옷판매', function(me, ch)
                    return me:dialog(ch, '특별 이벤트 기간에만 가능합니다.', { prev = false, next = true })
                end },
                { '호박결정만들기', function(me, ch)
                    return me:dialog(ch, '호박결정 만들기는 추가 구현이 필요합니다.', { prev = false, next = true })
                end },
            },
        })
    end
}
