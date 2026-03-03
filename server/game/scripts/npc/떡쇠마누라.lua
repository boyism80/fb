local ITEM_FRUIT_SALAD = '과일화채'
local REPAIRS = {
    { input = '웨딩드레스1', output = '웨딩드레스5' },
    { input = '웨딩드레스2', output = '웨딩드레스6' },
    { input = '웨딩드레스3', output = '웨딩드레스7' },
}

function NPC_237(me, npc)
    local btn = me:dialog(npc, '아이구 더워. 너무 더워서 뭔 일을 할 수가 없네...', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, '과일화채와 웨딩드레스1,2,3 중의 하나를 주실려우?', {
        '예, 웨딩드레스1을 수선해주세요.',
        '예, 웨딩드레스2를 수선해주세요.',
        '예, 웨딩드레스3을 수선해주세요.',
        '아니오. 다음에 다시 오겠습니다.',
    }, false)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel == 3 then
        return
    end
    if sel < 0 or sel > 2 then
        return
    end

    local r = REPAIRS[sel + 1]
    local code = me:exchange(
        { ['item'] = { [ITEM_FRUIT_SALAD] = 1, [r.input] = 1 } },
        { ['item'] = { [r.output] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '아이고, 더운데 재료도 없고 신경질나게 하지마!', false, false)
        return
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 수선한 드레스를 받을 수 없어요.', false, false)
        return
    end
    me:dialog(npc, '여기 수선이 끝났으니 가지고 가시우... 잘 입으시구랴...', false, true)
end