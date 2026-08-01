-- npc: 뭉치
local npc = require('lib.npc')

local SELL_CATEGORIES = {
    { '전사용 갑주류', 11 },
    { '전사용 비늘갑주류', 12 },
    { '도적용 도복류', 13 },
    { '도적용 갑옷류', 14 },
    { '주술사용 치마류', 15 },
    { '주술사용 도포류', 16 },
    { '도사용 장삼류', 17 },
    { '남자용 투구류', 18 },
}

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch, SELL_CATEGORIES)
                end },
                { '물건 팔기', function(me, ch)
                    return npc.show_buy_menu(me, ch)
                end },
            },
        })
    end
}
