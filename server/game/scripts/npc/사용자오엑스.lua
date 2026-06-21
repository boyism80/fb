local npc = require('lib.npc')
function NPC_309(me, npc_obj)
    local sel, btn = me:list(npc_obj, "무엇을 도와드릴까요?", {
        "물건 사기",
        "아무나오엑스입장",
        "이벤트오엑스입장",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if sel == 1 then
        local map = name2map("아무나오엑스시작")
        if map then
            me:map(map, math.random(13, 15), math.random(2, 4))
        else
            me:dialog(npc_obj, "입장할 수 있는 장소가 없습니다.", false, false)
        end
        return
    end

    if sel == 2 then
        if property("oxquiz_open") == 1 then
            local map = name2map("OX퀴즈장")
            if map then
                me:map(map, math.random(13, 15), math.random(2, 4))
            else
                me:dialog(npc_obj, "입장할 수 있는 장소가 없습니다.", false, false)
            end
        else
            me:dialog(npc_obj, "OX퀴즈 입장 시간이 지나 입장하실 수 없습니다.", false, false)
        end
    end
end
