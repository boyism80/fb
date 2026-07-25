-- npc: 배심원투표도우미
local quest = require('lib.quest')
local server = require('lib.server')

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_JURY)

        if q ~= nil and q:step() == 1 then
            local sel, list_btn = me:list(npc, "당신은 투표를 모두 마치셨군요. 귀환하시겠습니까?", {
                "예. 지금 귀환하겠습니다.",
                "아니오. 조금 더 지켜보겠습니다.",
            }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                me:dialog(npc, "안녕히 가십시오.", { prev = false, next = true })
                if not me:has_items("노란비서", 1) then
                    me:dialog(npc, "만일의 사태를 대비하기 위해 노란비서를 지참해 주시기 바랍니다.", { prev = false, next = false })
                    return
                end
                if not me:rmitem("노란비서", 1, ITEM_DELETE_TYPE.REDUCE) then
                    return
                end
                q:step(0)
                if server.warp_to_return_map(me) == nil then
                    me:dialog(npc, "이동할 수 없습니다.", { prev = false, next = false })
                end
            elseif sel == 2 then
                me:dialog(npc, "남아서 다른 의견을 더 들어보는 것도 좋은 일이지요.", { prev = false, next = true })
            end
            return
        end

        local btn = me:dialog(npc, "안녕하세요? 저는 배심원으로 참여한 여러분의 투표를 도와드릴 배심원투표도우미라고 합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "배심원 투표는 사적인 친분이나 이해관계에 따라 이뤄지면 안되며, 반드시 공정한 저울의 눈금 위에 있어야만 합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "따라서 모든 배심원 분들은 다음 선서를 읽고 이해하시기 바랍니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "선서 첫번째", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "사실을 정당하게 판단할 것과 재판장이 설명하는 법과 증거에 의하여 진실하게 평결할 것을 엄숙히 선서합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "선서 두번째", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "모든 사실을 공정하고 정당하게 판결할 것이며, 만약 그러할 자신이 없다면 기권할 것을 엄숙히 선서합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local sel, list_btn = me:list(npc, "그럼, 모든 재판의 내용을 이해하였고, 선서에 동의하십니까?", {
            "예. 동의합니다.",
            "아니오. 동의하지 않습니다.",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 2 then
            me:dialog(npc, "재판정은 당신의 의견을 존중합니다. 그러나 배심원 투표에 있어서는 당신의 참여는 투표에 적합하지 않은 것 같군요.", { prev = false, next = false })
            return
        end

        btn = me:dialog(npc, "좋습니다. 그렇다면 이제 투표 절차를 시작하겠습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "사적 친분이나 이해관계에 의한 투표가 아닌, 공정하고 진실하게 평결해주시기 바랍니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local vote_sel, vote_btn = me:list(npc, "어느쪽이 더 잘못했다고 생각하십니까?", {
            "좌측(왼쪽)의 사람이 잘못했습니다.",
            "우측(오른쪽)의 사람이 잘못했습니다.",
            "판단할 자신이 없군요. 기권합니다.",
        }, { prev = false })
        if vote_btn == DIALOG_RESULT.QUIT or vote_sel == nil then
            return
        end
        if q == nil then
            q = me:start_quest(quest.QUEST_JURY)
            if q == nil then
                return
            end
        end
        if q then
            q:step(1)
        end
        me:dialog(npc, "당신의 의견이 반영되었습니다.", { prev = false, next = true })
    end
}
