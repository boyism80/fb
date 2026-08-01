-- npc: 열쇠교환상
local enum = require('lib.enum')

local EXCHANGES = {
    { key = '보물열쇠1', item = '현철장갑', clothes = '장갑' },
    { key = '보물열쇠2', item = '속삭임팔찌', clothes = '팔찌' },
    { key = '보물열쇠3', item = '진마반지', clothes = '반지' },
    { key = '보물열쇠4', item = '선류', clothes = '선류' },
}

return {
    on_click = function(me, npc)
        ::NPC_214_COS001::
        local sel = me:list(npc, '아함~ 졸려.... 무슨일로 왔는가? 난 바쁜사람이라네...', { '보물열쇠를 가지고 왔어요...', '하하.. 그냥 인사차...' }, { prev = false })
        if sel == nil then
            return
        end

        if sel == 2 then
            me:dialog(npc, '허허.. 싱겁기는....', { prev = false, next = false })
            return
        end

        if sel ~= 1 then
            return
        end

        local btn = me:dialog(npc, '오호 그런가? 보물열쇠를 가지고 오면 내가 바꿔주긴 하는데...', { prev = false, next = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local key_options = {}
        for i = 1, 4 do
            key_options[i] = name_with(EXCHANGES[i].key, '을', '를') .. ' 가지고 왔습니다.'
        end
        local key_opt = me:list(npc, '그래... 무슨 열쇠를 가지고 왔나?', key_options, { prev = false })
        if key_opt == nil then
            return
        end
        if key_opt < 0 or key_opt > 3 then
            return
        end

        local ex = EXCHANGES[key_opt]
        local code = me:exchange(
            { ['item'] = { [ex.key] = 1 } },
            { ['item'] = { [ex.item] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, name_with(ex.key, '이', '가') .. ' 없지 않은가?!', { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(ex.item, '을', '를') .. ' 받을 수 없습니다.', { prev = false, next = false })
            return
        end
        me:dialog(npc, '오호 ' .. name_with(ex.key, '을', '를') .. ' 정말로 가지고 왔구먼, 내 좋은 ' .. name_with(ex.clothes, '을', '를') .. ' 선물로 줌세나', { prev = false, next = false })
    end
}
