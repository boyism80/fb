-- npc: 암표상
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local TICKET_PRICE = 8000

        ::NPC_53_0001::
        local button = me:dialog(npc, '일본까지 가시려고? 내 표한장 드릴까? 싸게해드릴께..', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_53_0002::
        local selected, button = me:list(npc, '부여-일본쾌속승선권 한장 사시려나?', {'네, 한장 주세요.', '혹시 사기꾼 아냐?'}, { prev = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_53_0001
        end

        if selected == 1 then
            local selected2, button2 = me:list(npc, '부여-일본쾌속승선권.. 한장에 8000원에 드리지. 어때?', {'8000원에 사죠.', '너무 비싼데...'}, { prev = false })
            if button2 == DIALOG_RESULT.QUIT then
                return
            end

            if selected2 == 1 then
                button = me:dialog(npc, '그래? 그럼 표 한장 드릴께..', { prev = false, next = true })
                if button == DIALOG_RESULT.QUIT then
                    return
                end

                local code = me:exchange(
                    { ['money'] = TICKET_PRICE },
                    { ['item'] = { ['부여-일본쾌속승선권'] = 1 } }
                )
                if code == enum.EXCHANGE_RESULT.LACK_COST then
                    me:dialog(npc, '이 사람이! 돈이 없잖아 돈이!', { prev = false, next = true })
                    return
                end
                if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                    me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', { prev = false, next = true })
                    return
                end
                me:dialog(npc, '그럼 일본 잘갔다와~ 나중에 또 보자고..', { prev = false, next = true })
                return
            elseif selected2 == 2 then
                ::NPC_53_0003::
                button = me:dialog(npc, '너무 비싸다고? 얼마까지 알아보고 왔어? 에이.. 하나도 안 남는데..', { prev = false, next = true })
                if button == DIALOG_RESULT.QUIT then
                    return
                end

                ::NPC_53_0004::
                button = me:dialog(npc, '에이.. 좋다. 그럼 내 노란비서 하나 끼워 준다. 그럼 됐지?', { prev = true, next = true })
                if button == DIALOG_RESULT.QUIT then
                    return
                end
                if button == DIALOG_RESULT.PREV then
                    goto NPC_53_0003
                end

                ::NPC_53_0005::
                local selected3, button3 = me:list(npc, '살꺼지?', {'그러죠 그럼..', '안살래요.'}, { prev = true })
                if button3 == DIALOG_RESULT.QUIT then
                    return
                end
                if button3 == DIALOG_RESULT.PREV then
                    goto NPC_53_0004
                end

                if selected3 == 1 then
                    ::NPC_53_0006::
                    button = me:dialog(npc, '그래그래. 잘 생각한거야. 그럼 볼까나....', { prev = false, next = true })
                    if button == DIALOG_RESULT.QUIT then
                        return
                    end

                    local code = me:exchange(
                        { ['money'] = TICKET_PRICE },
                        { ['item'] = { ['노란비서'] = 1, ['부여-일본쾌속승선권'] = 1 } }
                    )
                    if code == enum.EXCHANGE_RESULT.LACK_COST then
                        me:dialog(npc, '이 사람이! 돈이 없잖아 돈이!', { prev = false, next = true })
                        return
                    end
                    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                        me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', { prev = false, next = true })
                        return
                    end
                    me:dialog(npc, '그럼 일본 잘갔다와~ 나중에 또 보자고..', { prev = false, next = true })
                    return
                elseif selected3 == 2 then
                    me:dialog(npc, '관심없음 그냥 가구.. 다른데 알아봐. 더 싼데 있나..', { prev = false, next = true })
                    return
                end
            end
        elseif selected == 2 then
            me:dialog(npc, '어허.. 이 사람 보게? 안사면 안사는거지 왜 사람을 사기꾼으로 몰고 그래!', { prev = false, next = true })
            return
        end
    end
}
