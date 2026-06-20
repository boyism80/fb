local npc = require('lib.npc')
local server = require('lib.server')

local CHANGEITEM = {
    { source = { name = '낡은어전의칼', count = 10 },   dest = { name = '어전의칼', count = 1 } },
    { source = { name = '원숭이잔털', count = 10 },    dest = { name = '원숭이털', count = 1 } },
    { source = { name = '너구리잔털', count = 10 },    dest = { name = '너구리털', count = 1 } },
    { source = { name = '은여우잔털', count = 10 },    dest = { name = '은여우털', count = 1 } },
    { source = { name = '낡은도깨비방망이', count = 10 }, dest = { name = '도깨비방망이', count = 1 } },
    { source = { name = '참나무껍질', count = 10 },    dest = { name = '참나무조각', count = 1 } },
    { source = { name = '깨진불의수정', count = 10 },  dest = { name = '불의수정', count = 1 } },
    { source = { name = '낡은철도', count = 10 },      dest = { name = '철도', count = 1 } },
    { source = { name = '늑대의큰송곳니', count = 1 }, dest = { name = '늑대의송곳니', count = 2 } },
    { source = { name = '다랑어살코기뭉치', count = 1 }, dest = { name = '다랑어살코기', count = 10 } },
    { source = { name = '노회뭉치', count = 1 },       dest = { name = '노회', count = 10 } }
}

local function run_change_item(me, npc_obj)
    local list_msg = '안쓰는 물건을 비슷한 종류의 다른 물건으로 바꿔주는 소일거리를 하고 있어요. 혹시 이런 물건 가지고 있으세요?'
    local list_options = {}
    for _, e in ipairs(CHANGEITEM) do
        list_options[#list_options + 1] = e.source.name
    end
    local selected = me:list(npc_obj, list_msg, list_options)
    if selected == nil then
        return false
    end
    local idx = selected + 1
    if idx < 1 or idx > #CHANGEITEM then
        return true
    end

    local entry = CHANGEITEM[idx]
    local src = entry.source
    local dest = entry.dest

    local confirm_msg = string.format('%s %d개를 가져오시면 %s %d개로 교환해 드려요. 하시겠어요?', src.name, src.count, dest.name, dest.count)
    local confirm = me:list(npc_obj, confirm_msg, { '예', '아뇨' })
    if confirm == nil then
        return false
    end
    if confirm ~= 0 then
        return true
    end

    local code = me:exchange(
        { ['item'] = { [src.name] = src.count } },
        { ['item'] = { [dest.name] = dest.count } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc_obj, string.format('%s %d개를 가져오셔야 바꿔드려요.', src.name, src.count))
        return true
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc_obj, '소지품이 가득 차서 받을 수 없어요.', false, false)
        return true
    end
    return true
end

function NPC_231(me, npc_obj)
::NPC_231_000::
    local selected = me:list(npc_obj, '안녕하세요. 어떻게 오셨나요?', {
        '물건 사기',
        '물건 팔기',
        '금전 맡기기',
        '금전 되찾기',
        '물건 맡기기',
        '물건 되찾기',
        '안쓰는 물건 교환'
    })
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_231_000
        end
    elseif selected == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_231_000
        end
    elseif selected == 2 then
        if npc.hold_money_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_231_000
        end
    elseif selected == 3 then
        if npc.return_money_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_231_000
        end
    elseif selected == 4 then
        if npc.hold_item_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_231_000
        end
    elseif selected == 5 then
        if npc.return_item_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_231_000
        end
    elseif selected == 6 then
        if run_change_item(me, npc_obj) then
            goto NPC_231_000
        end
    end
end