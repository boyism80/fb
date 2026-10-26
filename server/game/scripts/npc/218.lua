-- npc: 장안성대장간
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
            },
        })
    end,

    on_black_flag = function(me, npc_obj)
        ::NPC_218_0001::
        if me:dialog(npc_obj, '아니, 내가 검정깃발을 가지고 있다는걸 어떻게 알았나.. 으음...', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        local button = me:dialog(npc_obj, '그냥 줄순 없고.. 5000전만 내게. 그럼 검정깃발을 하나 주지.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_218_0001
        end
        local selected, list_button = me:list(npc_obj, '어때? 5000전에 검정깃발 하나 사길 텐가?', { '네, 주십시오.', '안 살래요' })
        if list_button == DIALOG_RESULT.QUIT then
            return
        end
        if selected ~= 1 then
            return
        end
        local black_flag_price = 5000
        if me:money() < black_flag_price then
            me:dialog(npc_obj, '돈이 모자랍니다.', { prev = false, next = true })
            return
        end
        if me:mkitem('검정깃발', 1) == nil then
            me:dialog(npc_obj, '공간이 부족합니다.', { prev = false, next = true })
            return
        end
        me:money(me:money() - black_flag_price)
        me:dialog(npc_obj, '검정깃발을 받았습니다.', { prev = false, next = false })
    end
}
