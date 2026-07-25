-- npc: 백호제단지기

return {
    ON_CLICK = function(me, npc)
        local d = me:dialog(npc, "안녕하신가, " .. me:name() .. ". 나는 이곳 제단에서 힘을 모아, 사악한악령에게 [민첩한 공격]을 가하는 백호제단지기라네.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        d = me:dialog(npc, " 내가 천도복숭아를 먹고, 일격을 가할만한 힘이 [1250 마력]만큼 모이게 되면,\n [민첩한 공격]을 사악한 악령에게 가하여 데미지를 4 입힐 수 있다네.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end

        local list_msg = "그대는 나에게 바라는 것이 무엇인가?\n\n나의 백호제단 기여순위: -위\n현재 백호제단 기여총점: -/1250 마력"
        local sel, btn = me:list(npc, list_msg, {
            "백호제단의 능력치 보기 (공격 능력치)",
            "백호제단의 기여자 조회 (상위 100명)",
            "백호제단지기에게 천도복숭아 바치기",
            "기여자 보상 수령",
        }, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            me:dialog(npc, "천상계에서 난다는 천도복숭아는, 우리 반신들의 능력을 강화시켜 준다네.", { prev = false, next = false })
            return
        end
        if sel == 2 then
            me:dialog(npc, "기여자 조회는 준비중입니다.", { prev = false, next = false })
            return
        end
        if sel == 3 then
            me:dialog(npc, "천도복숭아 바치기는 준비중입니다.", { prev = false, next = false })
            return
        end
        if sel == 4 then
            me:dialog(npc, "기여자 보상 수령은 준비중입니다.", { prev = false, next = false })
            return
        end
    end
}
