-- npc: 복불복도우미
local server = require('lib.server')
function NPC_340(me, npc)
    if me:role() >= ROLE.ADMIN then
        local sel, btn = me:list(npc, "어떤 명령을 수행하시겠습니까?", {
            "몬스터생성",
            "출입구닫기",
            "초기화",
        }, { prev = false })
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    local sel, btn = me:list(npc, "귀환 하시겠습니까?", {
        "네. 나가고 싶어요!",
        "아니요. 아직 안나갈래요!",
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 1 then
        if server.warp_to_return_map(me) == nil then
            me:dialog(npc, "이동할 수 없습니다.", { prev = false, next = false })
        end
    elseif sel == 2 then
        me:dialog(npc, "잘 생각하셨어요.", { prev = false, next = true })
    end
end