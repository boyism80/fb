-- npc: 사용자오엑스
local npc = require('lib.npc')

function NPC_309(me, npc_obj)
    npc.shop(me, npc_obj, {
        greeting = '무엇을 도와드릴까요?',
        menu = {
            { '물건 사기', function(me, ch)
                return npc.show_sell_menu(me, ch)
            end },
            { '아무나오엑스입장', function(me, ch)
                local map = name2map('아무나오엑스시작')
                if map then
                    me:map(map, math.random(13, 15), math.random(2, 4))
                    return DIALOG_RESULT.NEXT
                end
                return me:dialog(ch, '입장할 수 있는 장소가 없습니다.', { prev = false, next = true })
            end },
            { '이벤트오엑스입장', function(me, ch)
                if property('oxquiz_open') == 1 then
                    local map = name2map('OX퀴즈장')
                    if map then
                        me:map(map, math.random(13, 15), math.random(2, 4))
                        return DIALOG_RESULT.NEXT
                    end
                    return me:dialog(ch, '입장할 수 있는 장소가 없습니다.', { prev = false, next = true })
                end
                return me:dialog(ch, 'OX퀴즈 입장 시간이 지나 입장하실 수 없습니다.', { prev = false, next = true })
            end },
        },
    })
end
