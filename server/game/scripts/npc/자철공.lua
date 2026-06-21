local quest = require('lib.quest')
local server = require('lib.server')
local REQUIRED_ITEMS = { ["자철석"] = 1, ["무슈후슈의뿔"] = 10 }

function NPC_562(me, npc)
    local q = me:quest(quest.QUEST_BLUEEAR)

    if q == nil then
        ::NPC_562_0000::
        local sel, list_btn = me:list(npc, "무슨일로 나를 찾아왔는가?", {
            "지남침에 대해 알고 싶습니다.",
            "지남침을 구해주십시오.",
            "어떤 갑옷을 파시는지요?",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            local dialog_btn = me:dialog(npc, "지남침은 나침반이라고도 하는 물건으로 안에 들어있는 침이 언제 어디서든 남북을 가리키고 있다오.", false, true)
            if dialog_btn == DIALOG_RESULT.QUIT then
                return
            end
            if dialog_btn == DIALOG_RESULT.PREV then
                goto NPC_562_0000
            end
            return
        end
        if sel == 2 then
            me:dialog(npc, "갑옷은 팔지 않네!!", false, false)
            return
        end
        if sel == 1 then
            ::NPC_562_0001::
            local dialog_btn = me:dialog(npc, "지남침을 구하고 싶다고? 만드는것은 그렇게 어려운일이 아닐세. 시간도 별로 걸리지 않고.", true, true)
            if dialog_btn == DIALOG_RESULT.QUIT then
                return
            end
            if dialog_btn == DIALOG_RESULT.PREV then
                goto NPC_562_0000
            end
            ::NPC_562_0002::
            dialog_btn = me:dialog(npc, "다만 그 재료가 상당히 희귀하기 때문에 지남침이 귀한것이지. 우선 자철석이 필요하다네.", true, true)
            if dialog_btn == DIALOG_RESULT.QUIT then
                return
            end
            if dialog_btn == DIALOG_RESULT.PREV then
                goto NPC_562_0001
            end
            ::NPC_562_0003::
            dialog_btn = me:dialog(npc, "신기하게도 중국의 괴물중에 괴성들의 우두머리의 몸에 질좋은 자철석이 생긴다고 하더군.", true, true)
            if dialog_btn == DIALOG_RESULT.QUIT then
                return
            end
            if dialog_btn == DIALOG_RESULT.PREV then
                goto NPC_562_0002
            end
            ::NPC_562_0004::
            dialog_btn = me:dialog(npc, "그리고 그 자철석을 담을 용기를 만들려면 무슈후슈의 뿔이 필요하지.", true, true)
            if dialog_btn == DIALOG_RESULT.QUIT then
                return
            end
            if dialog_btn == DIALOG_RESULT.PREV then
                goto NPC_562_0003
            end
            ::NPC_562_0005::
            dialog_btn = me:dialog(npc, "괴성왕에게서 자철석을, 무슈후슈에게서 뿔 10개를 구해오면 내 만들어주겠네.", false, true)
            if dialog_btn == DIALOG_RESULT.QUIT then
                return
            end
            if dialog_btn == DIALOG_RESULT.PREV then
                goto NPC_562_0004
            end
            q = me:start_quest(quest.QUEST_BLUEEAR)
            if q == nil then
                return
            end
            if q then
                q:step(1)
            end
        end
        return
    end

    if q:completed() then
        me:dialog(npc, "또 필요하다면 언제든지 오게.", false, false)
        return
    end

    if q:step() == 1 then
        if not me:has_items(REQUIRED_ITEMS) then
            me:dialog(npc, "괴성왕에게서 자철석을, 무슈후슈에게서 뿔 10개를 구해오면 내 만들어주겠네.", false, false)
            return
        end
        ::NPC_562_0010::
        local dialog_btn = me:dialog(npc, "오 드디어 다 구해왔군. 그럼 지남침을 만들어 주겠네.", false, true)
        if dialog_btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_562_0011::
        dialog_btn = me:dialog(npc, "자 다 만들었네. 자네가 고생했지 난 별로 한게 없구려. 또 필요하다면 언제든지 오게.", false, true)
        if dialog_btn == DIALOG_RESULT.QUIT then
            return
        end
        if dialog_btn == DIALOG_RESULT.PREV then
            goto NPC_562_0010
        end
        local code = me:exchange(
            { ['item'] = REQUIRED_ITEMS },
            { ['item'] = { ["지남침"] = 1 } }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "재료가 부족합니다.", false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 지남침을 드리지 못합니다. 재료는 돌려드리겠네.", false, false)
            return
        end
        q:complete()
        return
    end
end
