-- npc: 채희
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, '시원한 과일 화채를 만들어 드립니다.', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local cost = { ['홍옥'] = 5, ['국광'] = 5, ['깨끗한얼음'] = 10, ['참수박'] = 2 }
        local code = me:exchange(
            { ['item'] = cost },
            { ['item'] = { ['과일화채'] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '과일 화채의 재료는 홍옥 5개, 국광 5개, 깨끗한얼음 10개, 참수박 2개가 필요합니다.', { prev = false, next = true })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 과일화채를 받을 수 없습니다.', { prev = false, next = true })
            return
        end
        me:dialog(npc, '정말 시원하고 맛있는 과일화채가 여기 나왔습니다.', { prev = false, next = true })
    end
}
