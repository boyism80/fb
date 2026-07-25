-- npc: 북방나무꾼
local quest = require('lib.quest')
local enum = require('lib.enum')
local COOLDOWN_SEC = 82800

local function next_available(me)
    local q = me:quest(quest.QUEST_JANGAJI)
    if q == nil then
        return 0
    end
    local p = q:param()
    if p == nil or p == "" then
        return 0
    end
    return tonumber(p) or 0
end

local function set_cooldown(me)
    local now_ts = now()
    if not now_ts or now_ts <= 0 then
        return
    end
    local q = me:quest(quest.QUEST_JANGAJI)
    if q == nil then
        q = me:start_quest(quest.QUEST_JANGAJI)
        if q == nil then
            return
        end
    end
    if q then
        q:param(tostring(now_ts + COOLDOWN_SEC))
    end
end

local function do_exchange(me, npc, need_count, reward_name)
    local code = me:exchange(
        { ['item'] = { ["잔가지"] = need_count } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "잔가지를 몇개만 주워다 주시면 제가 가진 나뭇가지들을 드리겠습니다. 잘 부탁 드리겠습니다.", { prev = false, next = false })
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 나무를 줄 수 없습니다.", { prev = false, next = false })
        return
    end
    set_cooldown(me)
    me:dialog(npc, "앗, 정말로 잔가지들을 구해오셨군요! 그럼 여기 제가 가진 " .. reward_name .. "을 하나 드리겠습니다. 앞으로도 잘 부탁 드리겠습니다.", { prev = false, next = false })
end

return {
    on_click = function(me, npc)
        local now_ts = now()
        local next_ts = next_available(me)
        if now_ts and now_ts > 0 and next_ts > now_ts then
            local rem = next_ts - now_ts
            local hour = math.floor(rem / 3600)
            local min = math.floor((rem % 3600) / 60)
            me:dialog(npc, string.format("일전에는 정말 감사했습니다. 곧 잔가지가 다시 필요할 것 같으니 %d시간 %d분 후에 다시 와주세요!", hour, min), { prev = false, next = false })
            return
        end

        local btn = me:dialog(npc, string.format("앗...! 안녕하십니까 %s님. 저는 북방나무꾼이라고 합니다.", me:name()), { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_450_0010::
        local sel, list_btn = me:list(npc, "북방대초원에는 어인일로 가시는지요...?", { "당신은 알 것 없네.", "나무를 좀 구하러..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == 1 then
            me:dialog(npc, "아아.. 그렇습니까. 부디 몸 조심하시길 바랍니다.", { prev = false, next = false })
            return
        end
        if sel ~= 2 then
            return
        end

        ::NPC_450_0011::
        btn = me:dialog(npc, "나무... 나무 말이군요!\n\n북방대초원에는 기괴한 괴물들이 나무를 모두 훔쳐가버리곤 한다는 사실... 알고 계셨습니까?", { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_450_0010
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_450_0020::
        btn = me:dialog(npc, "무시무시한 녀석들이 대체 어디에 쓰려는건지는 모르겠지만... \n\n귀한 [백현목], [자현목], [노송의가지]를 훔쳐가서는 애지중지 품고 다니곤 하더군요.", { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_450_0011
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_450_0021::
        sel, list_btn = me:list(npc, "혹시 나무를 구하러 가시는거라면, 제게 몇개 있는데...", { "갈길이 바빠서...", "앗, 나무가 있으시다구요?" }, { prev = true })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_450_0020
        end
        if sel == 1 then
            me:dialog(npc, "아아.. 그렇습니까. 부디 몸 조심하시길 바랍니다.", { prev = false, next = false })
            return
        end
        if sel ~= 2 then
            return
        end

        btn = me:dialog(npc, "그렇습니다. 제가 살고있는 집은, 북방의 초원에서 산으로 향하는 기슭에 위치하고 있습니다.\n\n때문에 땔감이 많이 필요한데, 이 귀한 나무들을 땔감으로 쓰기엔 뭣하고...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_450_0030::
        btn = me:dialog(npc, "[잔가지]정도면 좋을것 같다는 생각이 듭니다. \n\n그래서 잔가지들을 조금 구해다 주시면, 제가 가진 나무와 바꾸어드릴수 있을 것 같은데... ", { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_450_0021
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local opts = {
            "백현목     - 잔가지 100개를 가져왔습니다.",
            "자현목     - 잔가지 200개를 가져왔습니다.",
            "노송의가지 - 잔가지 500개를 가져왔습니다.",
            "잔가지는 어떻게 구하죠?",
        }
        sel, list_btn = me:list(npc, "잔가지를 가져오시면, 제가 가진 귀한 나뭇가지들과 교환 해 드리겠습니다.", opts, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_450_0030
        end

        if sel == 1 then
            do_exchange(me, npc, 100, "백현목")
            return
        end
        if sel == 2 then
            do_exchange(me, npc, 200, "자현목")
            return
        end
        if sel == 3 then
            do_exchange(me, npc, 500, "노송의가지")
            return
        end
        if sel == 4 then
            local map = me:map()
            local map_name = (map and map:model()) and map:model():name() or "이곳"
            me:dialog(npc, string.format("잔가지는 이곳 [%s]에서 나무를 베어 얻으실 수 있습니다.", map_name), { prev = false, next = true })
            me:dialog(npc, "어디까지나 자잘한 가지들을 모으는 것이기 때문에, 곳곳에 있는 나무들에게 다가가 잔가지를 주워 오시면 됩니다. ", { prev = false, next = true })
            me:dialog(npc, "나무들은 맨손으로 공격하셔도 좋고 마법으로 공격하셔도 좋습니다.\n\n그렇게 나무 주위에서 떨어지는 잔가지들을 구해다 주시면 됩니다...", { prev = false, next = false })
            return
        end
    end
}
