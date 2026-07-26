-- npc: 톨루이
local quest = require('lib.quest')
local enum = require('lib.enum')
local DOTOL_NAMES = {
    "황도톨", "적도톨", "연녹도톨", "녹도톨", "청도톨", "회도톨", "갈도톨", "연갈도톨",
    "연자도톨", "자도톨", "연청도톨", "황금도톨", "황왕도톨", "적왕도톨", "연녹왕도톨", "녹왕도톨",
    "청왕도톨", "회왕도톨", "갈왕도톨", "연갈왕도톨", "연자왕도톨", "자왕도톨", "연청왕도톨", "황금왕도톨",
}

local DOTOL_PRICE = 1000

return {
    on_click = function(me, npc)
        local btn = me:dialog(npc, "안녕하세요?\n\n저는 여러분에게 행운을 가져다 주는 톨루이 입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local day = datetime().day
        if day >= 1 and day <= 24 then
            ::NPC_336_0001::
            btn = me:dialog(npc, "저는 여러분에게 매일 다른 색깔의 도톨을 판매하고 있습니다.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_336_0002::
            btn = me:dialog(npc, "매달 1일부터 24일간 24개의 도톨을 종류별로 판매하고 있고,\n\n만약 놓치신 색도톨이 있다면 25일부터 26일 2일동안은 5만전을 주시면 원하는 색상의 도톨 한개를 구입하실 수 있습니다.", { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_336_0001
            end

            local q = me:quest(quest.QUEST_TOLUI_DAY)
            local last_day = (q and q:param() ~= "") and tonumber(q:param()) or 0
            if last_day == day then
                me:dialog(npc, "오늘은 이미 도톨을 구입하셨군요! 내일 다시 오세요.", { prev = false, next = false })
                return
            end

            local item_name = DOTOL_NAMES[day]
            if not item_name then
                me:dialog(npc, "준비중입니다.", { prev = false, next = false })
                return
            end
            btn = me:dialog(npc, "오늘은 [" .. item_name .. "] " .. name_with(item_name, '을', '를') .. " 구입 가능합니다.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local sel, list_btn = me:list(npc, "[" .. item_name .. "] " .. name_with(item_name, '을', '를') .. " 1천전에 구입하시겠습니까?", { "예", "아니오" }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
                return
            end
            if me:has_items(item_name, 1) then
                me:dialog(npc, "이미 [" .. item_name .. "] " .. name_with(item_name, '을', '를') .. " 가지고 계신것 같군요. 저는 욕심쟁이를 좋아하지 않아요.", { prev = false, next = false })
                return
            end
            if me:money() < DOTOL_PRICE then
                me:dialog(npc, "금전 천전이 없으시군요!", { prev = false, next = false })
                return
            end
            if q == nil then
                q = me:start_quest(quest.QUEST_TOLUI_DAY)
                if q == nil then
                    me:dialog(npc, "퀘스트를 시작할 수 없습니다.", { prev = false, next = false })
                    return
                end
            end
            me:money(me:money() - DOTOL_PRICE)
            me:mkitem(item_name, 1)
            if q then
                q:param(tostring(day))
            end
            me:dialog(npc, item_name .. " 받았습니다.", { prev = false, next = false })
            return
        end

        if day == 25 or day == 26 then
            me:dialog(npc, "오늘은 원하는 색상의 도톨 하나를 드리고 있어요. 단...! 가격은 비싸다는 점! 참고해 주세요. (해당 기능은 준비중입니다.)", { prev = false, next = false })
            return
        end

        local has_all = true
        for i = 1, 24 do
            if not me:has_items(DOTOL_NAMES[i], 1) then
                has_all = false
                break
            end
        end
        if not has_all then
            me:dialog(npc, "스물네가지 색의 도톨을 모아오셨다면, 일주일간 사용이 가능한 [칠지도]라는 무기와 교환 해 드리겠습니다.", { prev = false, next = true })
            me:dialog(npc, "저런, 아쉽게도 도톨이 부족하시군요.", { prev = false, next = false })
            return
        end
        local sel, list_btn = me:list(npc, "스물 네가지 색의 도톨을 가져 오셨다면, 일주일간 사용이 가능한 [칠지도]라는 무기와 교환 해 드리겠습니다.", { "예", "아니오" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end
        local cost_items = {}
        for i = 1, 24 do
            cost_items[DOTOL_NAMES[i]] = 1
        end
        local code = me:exchange(
            { ['item'] = cost_items },
            { ['item'] = { ["칠지도"] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, "도톨을 건네지 못했습니다.", { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 칠지도를 받을 수 없습니다.", { prev = false, next = false })
            return
        end
        me:dialog(npc, "와, 정말 모든 도톨을 다 모아 오셨군요! 그럼 칠지도로 교환 해 드릴게요.", { prev = false, next = true })
        me:dialog(npc, "여기 있습니다. 잘 쓰시길 바라며, 수행이 필요 없을지라도 꾸준한 수행은 자신의 진의를 깨달을 수 있는 길입니다. 나무아미타불..", { prev = false, next = false })
    end
}
