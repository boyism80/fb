-- npc: 원주민주술사
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local suffixes = { '장갑', '팔찌', '반지', '선류', '방패', '투구' }
        local btn

        ::NPC_177_0001::
        btn = me:dialog(npc, '문신이 들어간 장신구와 재료를 가지고 오면 강력한 힘을 가진 주술을 장신구에 걸어주겠네.\n\n재료는 향료, 기름, 문신이 들어간 장신구일세.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_177_0002::
        local sel, button = me:list(npc, '어떤 장신구에 주술을 걸고 싶은가?', {
            '문신장갑에 주술을 걸어주십시오.',
            '문신팔찌에 주술을 걸어주십시오.',
            '문신반지에 주술을 걸어주십시오.',
            '문신선류에 주술을 걸어주십시오.',
            '문신방패에 주술을 걸어주십시오.',
            '문신투구에 주술을 걸어주십시오.',
        }, { prev = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_177_0001
        end
        if sel < 1 or sel > #suffixes then
            return
        end

        local suffix = suffixes[sel]
        local item_tainted = '문신' .. suffix
        local item_result = '주술' .. suffix

        ::NPC_177_0003::
        btn = me:dialog(npc, '조상신이시여. 조상의 넋을 기리는 문신을 새긴 이 물건에 당신의 힘을 실어주소서...', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local materials = {[item_tainted] = 1, ['향료'] = 1, ['기름'] = 1}
        local code = me:exchange(
            { ['item'] = materials },
            { ['item'] = { [item_result] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, '재료가 부족한 것 같은데?', { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(item_result, '을', '를') .. ' 줄 수 없네.', { prev = false, next = true })
            return
        end
        me:dialog(npc, item_tainted .. '에 주술을 거는 데 성공했네... 부디 뜻깊은 일에 사용하게나. 과거 이 섬을 폭염왕의 마수에서 잠시나마 구해 줬던 영웅처럼 말일세...', { prev = true, next = true })
    end
}
