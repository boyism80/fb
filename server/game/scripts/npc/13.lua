-- npc: 예쁜이
local npc = require('lib.npc')

local SELL_CATEGORIES = {
    { '전사용 갑주류', 2 },
    { '전사용 비늘갑주류', 3 },
    { '도적용 도복류', 4 },
    { '도적용 갑옷류', 5 },
    { '주술사용 치마류', 6 },
    { '주술사용 도포류', 7 },
    { '도사용 장삼류', 8 },
    { '여자용 투구류', 9 },
}

return {
    ON_CLICK = function(me, npc_obj)
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
