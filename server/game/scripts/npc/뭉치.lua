local npc = require('lib.npc')
local server = require('lib.server')

local AMBER_STAR = {
    { base = '황호박', label = '황호박별입니다.' },
    { base = '적호박', label = '적호박별입니다.' },
    { base = '녹호박', label = '녹호박별입니다.' },
    { base = '청호박', label = '청호박별입니다.' },
    { base = '회호박', label = '회호박별입니다.' },
    { base = '갈호박', label = '갈호박별입니다.' },
    { base = '자호박', label = '자호박별입니다.' },
}

local function run_amber_star(me, npc_obj)
    local options = {}
    for i = 1, 7 do
        options[i] = AMBER_STAR[i].label
    end
    local sel, btn = me:list(npc_obj, '어떤 색깔의 호박별을 만드시겠어요?', options, true)
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if btn == DIALOG_RESULT.PREV then
        return true
    end
    if sel == nil or sel < 0 or sel > 6 then
        return false
    end
    local p = AMBER_STAR[sel + 1]
    local gem = p.base .. '보석'
    local star = p.base .. '별'
    local code = me:exchange(
        { ['item'] = { [gem] = 5 } },
        { ['item'] = { [star] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc_obj, name_with(gem, '이', '가') .. ' 부족합니다.', false, false)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc_obj, '소지품이 가득 차서 ' .. name_with(star, '을', '를') .. ' 받을 수 없어요.', false, false)
        return
    end
    me:dialog(npc_obj, name_with(star, '을', '를') .. ' 만들어드렸습니다.', false, true)
    return false
end

function NPC_15(me, npc_obj)
::NPC_15_0001::
    local sel, btn = me:list(npc_obj, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기'}, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_15_0001
        end
    elseif sel == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_15_0001
        end
    else

    end
end

function NPC_16(me, npc_obj)
::NPC_16_0001::
    local sel, btn = me:list(npc_obj, '안녕하세요. 어떻게 오셨나요?', { '물건 사기', '물건 팔기', '호박별만들기' }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end
    if sel == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_16_0001
        end
    elseif sel == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_16_0001
        end
    elseif sel == 2 then
        if run_amber_star(me, npc_obj) then
            goto NPC_16_0001
        end
    end
end