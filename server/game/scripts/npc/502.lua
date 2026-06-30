-- npc: 백남인
local enum = require('lib.enum')
function NPC_502(me, npc)
    local sesi = property("sesi_rightnow")
    if sesi == 1 then
        local sel, list_btn = me:list(npc, " 어쩐일로 찾아오셨습니까?", {
            "정월 대보름이 뭐에요?",
            "떡국을 드릴께요, 드세요.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            local btn = me:dialog(npc, " 정월 대보름은 1월 15일 입니다. 말 그대로 1월의 보름이라는 뜻입니다. 1월을 정월이라고 하지요.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, " 정월 대보름에는 해가 뜨기전에 친구나 친척을 만나 이름을 부르고, 상대방이 대답하면 '내 더위 사세요~'라고 하면 그 해 여름동안 더위를 먹지 않는다고 하죠.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, " 대보름엔 찰밥을 먹고, 아이들은 바람개비를 가지고 노는 날 이기도 하고, 밤에 달이 뜨면 달을 보며 소원을 비는 것도 빼놓을 수 없는 풍습입니다.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, " 세시마을에서 달맞이를 할 수 있는 곳은 세시마을 남쪽 무덤을 지나 달맞이 다리를 건너가시면 달맞이 고개가 있지요.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, " 달맞이 하실땐 꼭 설빔을 입고 가세요..", false, false)
            return
        end
        if sel == 1 then
            local armor = (me:gender() == GENDER.MALE) and "남자설빔" or "여자설빔"
            local code = me:exchange(
                { ['item'] = { ["떡국"] = 1 } },
                { ['item'] = { [armor] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, " 이번해는 정말 바빠서 떡국 한 그릇도 먹지 못하고 있네요. 이거 참...", false, false)
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "설빔을 지급할 수 없습니다.", false, false)
                return
            end
            local btn = me:dialog(npc, " 허허.. 이런 감사합니다. 바빠서 올해 떡국을 어떻게 먹나 했더니.. 이렇게 전해주시다니 정말 감사합니다.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, " 어디보자.. 이거 저도 뭐 보답을 해드려야 할텐데..", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, " 설빔이 없으신가 보네요? 그럼 제가 준비한 설빔을 드리면 되겠네요.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, " 설빔을 잘 간직하셔서 다른 명절에 받으신 아이템들과 같이 잘 모아두세요..", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, " 설빔 잘 입으시고 한해동안 복 많이 받으세요 ~", false, false)
            return
        end
        return
    end

    if sesi == 7 then
        me:dialog(npc, "칠석에는 견우와 직녀가 만나는데.. 올해는 까마귀 숫자가 부족해서 오작교가 만들어질 수 없을것 같다는데... (해당 이벤트는 일정에 따라 진행됩니다.)", false, false)
        return
    end

    me:dialog(npc, "지금은 칠성당 관련 이벤트 기간이 아닙니다.", false, false)
end
