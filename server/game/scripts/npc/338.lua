-- npc: 귀환도우미
local server = require('lib.server')

return {
    on_click = function(me, npc)
        local sel, btn = me:list(npc, "귀환 하시겠습니까?", { "네. 나가고 싶어요!", "아니요. 아직 안나갈래요!" }, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            if not me:has_items("노란비서", 1) then
                me:dialog(npc, "만일의 사태를 대비하기 위해 노란비서를 지참해 주시기 바랍니다.", { prev = false, next = false })
                return
            end
            if server.warp_to_return_map(me, server.item_cost('scripts/npc/338.lua', "노란비서")) ~= true then
                me:dialog(npc, "이동할 수 없습니다.", { prev = false, next = false })
            end
        elseif sel == 2 then
            me:dialog(npc, "잘 생각하셨어요.", { prev = false, next = false })
        end
    end,

    on_before_warp = function(me, name)
        return server.pay_item(me, name)
    end,
}
