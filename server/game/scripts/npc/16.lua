-- npc: 떼보
local npc = require('lib.npc')
local enum = require('lib.enum')

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

local AMBER_STAR = {
    { base = '황호박', label = '황호박별입니다.' },
    { base = '적호박', label = '적호박별입니다.' },
    { base = '녹호박', label = '녹호박별입니다.' },
    { base = '청호박', label = '청호박별입니다.' },
    { base = '회호박', label = '회호박별입니다.' },
    { base = '갈호박', label = '갈호박별입니다.' },
    { base = '자호박', label = '자호박별입니다.' },
}

local function craft_amber_star(me, ch)
    local options = {}
    for i = 1, #AMBER_STAR do
        options[i] = AMBER_STAR[i].label
    end
    local sel, btn = me:list(ch, '어떤 색깔의 호박별을 만드시겠어요?', options, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return DIALOG_RESULT.QUIT
    end
    if btn == DIALOG_RESULT.PREV then
        return DIALOG_RESULT.NEXT
    end
    if sel == nil or sel < 1 or sel > 7 then
        return DIALOG_RESULT.QUIT
    end

    local p = AMBER_STAR[sel]
    local gem = p.base .. '보석'
    local star = p.base .. '별'
    local code = me:exchange(
        { ['item'] = { [gem] = 5 } },
        { ['item'] = { [star] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        return me:dialog(ch, name_with(gem, '이', '가') .. ' 부족합니다.', { prev = false, next = true })
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        return me:dialog(ch, '소지품이 가득 차서 ' .. name_with(star, '을', '를') .. ' 받을 수 없어요.', { prev = false, next = true })
    end
    return me:dialog(ch, name_with(star, '을', '를') .. ' 만들어드렸습니다.', { prev = false, next = true })
end

function NPC_16(me, npc_obj)
    npc.shop(me, npc_obj, {
        greeting = '안녕하세요. 어떻게 오셨나요?',
        menu = {
            { '물건 사기', function(me, ch)
                return npc.show_sell_menu(me, ch, SELL_CATEGORIES)
            end },
            { '물건 팔기', function(me, ch)
                return npc.show_buy_menu(me, ch)
            end },
            { '호박별만들기', function(me, ch)
                return craft_amber_star(me, ch)
            end },
        },
    })
end