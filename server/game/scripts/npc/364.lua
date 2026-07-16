-- npc: 혜방스님
local enum = require('lib.enum')
local function yeondeung_list()
    local opts = {}
    opts[1] = "연등 2개를 연등1로 교환 해 주세요."
    for i = 1, 30 do
        opts[i] = string.format("연등%d 2개를 연등%d로 교환 해 주세요.", i, i + 1)
    end
    opts[32] = "아뇨 됐습니다..."
    return opts
end

function NPC_364(me, npc)
    ::NPC_364_0001::
    local btn = me:dialog(npc, "안녕하십니까. 저는 명경사의 혜방이라고 합니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local opts = yeondeung_list()
    ::NPC_364_0002::
    local sel, list_btn = me:list(npc, "같은 연등을 두개 모아 오시면, 상위 연등으로 교환 해 드립니다..", opts, true)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto NPC_364_0001
    end
    if sel == nil then
        return
    end

    if sel == 32 then
        me:dialog(npc, "안녕히 가십시오...", false, false)
        return
    end

    local source_name = (sel == 1) and "연등" or ("연등" .. sel)
    local target_name = "연등" .. (sel + 1)

    local code = me:exchange(
        { ['item'] = { [source_name] = 2 } },
        { ['item'] = { [target_name] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "연등이 없으시군요....", false, false)
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 연등을 드리지 못합니다.", false, false)
        return
    end
    me:dialog(npc, "교환 해 드렸습니다. 필요없는 연등이 있으시면 언제든 저를 찾아 주세요...", false, false)
end