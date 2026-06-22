-- npc: 폭노폭노
local npc = require('lib.npc')
function NPC_487(me, npc_obj)
    ::NPC_487_000::
    local sel, btn = me:list(npc_obj, "안녕하세요. 어떻게 오셨나요?", {
        "물건 사기",
        "물건 팔기",
        "귀환",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.QUIT then
            return
        end
        goto NPC_487_000
    end
    if sel == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.QUIT then
            return
        end
        goto NPC_487_000
    end
    if sel == 2 then
        local map = name2map("국내성")
        if map then
            me:map(map, math.random(3, 8), math.random(5, 9))
        else
            me:dialog(npc_obj, "이동할 수 없습니다.", false, false)
        end
        return
    end
end
