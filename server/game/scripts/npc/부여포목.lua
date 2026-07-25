local npc = require('lib.npc')
local enum = require('lib.enum')

local AMBER_CRYSTAL_EXCHANGE = {
    { source = { name = '연갈호박단추', count = 15 }, dest = { name = '연갈호박결정', count = 1 } },
    { source = { name = '연녹호박단추', count = 15 }, dest = { name = '연녹호박결정', count = 1 } },
    { source = { name = '연자호박단추', count = 15 }, dest = { name = '연자호박결정', count = 1 } },
    { source = { name = '연청호박단추', count = 15 }, dest = { name = '연청호박결정', count = 1 } },
}

local function craft_amber_crystal(me, ch, exchange_list)
    local list_options = {}
    for _, e in ipairs(exchange_list) do
        list_options[#list_options + 1] = e.dest.name .. '입니다.'
    end
    local choice = me:list(ch, '어떤 색깔의 호박결정을 만드시겠어요?', list_options)
    if choice == nil then
        return DIALOG_RESULT.NEXT
    end

    local idx = choice
    if idx < 1 or idx > #exchange_list then
        return DIALOG_RESULT.NEXT
    end

    local entry = exchange_list[idx]
    local src = entry.source
    local dest = entry.dest
    local code = me:exchange(
        { ['item'] = { [src.name] = src.count } },
        { ['item'] = { [dest.name] = dest.count } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        return me:dialog(ch, name_with(src.name, '이', '가') .. ' 부족합니다.', { prev = false, next = true })
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        return me:dialog(ch, '소지품이 가득 차서 ' .. name_with(dest.name, '을', '를') .. ' 받을 수 없어요.', { prev = false, next = true })
    end
    return me:dialog(ch, name_with(dest.name, '을', '를') .. ' 만들어드렸습니다.', { prev = false, next = true })
end

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '안녕하세요. 어떻게 오셨나요?',
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
                { '물건 팔기', function(me, ch)
                    return npc.show_buy_menu(me, ch)
                end },
                { '끈옷판매', function(me, ch)
                    return me:dialog(ch, '특별 이벤트 기간에만 가능합니다.', { prev = false, next = true })
                end },
                { '산타클로스옷판매', function(me, ch)
                    return me:dialog(ch, '특별 이벤트 기간에만 가능합니다.', { prev = false, next = true })
                end },
                { '호박결정만들기', function(me, ch)
                    return craft_amber_crystal(me, ch, AMBER_CRYSTAL_EXCHANGE)
                end },
            },
        })
    end
}
