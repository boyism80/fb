-- npc: 흉노족행상인
local npc = require('lib.npc')

function NPC_488(me, npc_obj)
    npc.shop(me, npc_obj, {
        greeting = '이 위험한 곳까지 와서 장사하기가 쉬운 줄 아나?',
        menu = {
            { '물건 사기', function(me, ch)
                return npc.show_sell_menu(me, ch)
            end },
            { '물건 팔기', function(me, ch)
                return npc.show_buy_menu(me, ch)
            end },
        },
    })
end