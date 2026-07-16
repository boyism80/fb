-- npc: 장안성백성
local quest = require('lib.quest')
function NPC_539(me, npc)
    if me:level() < 50 then
        return
    end

    local q = me:quest(quest.QUEST_JINHWANG)

    if q == nil then
        ::NPC_539_0000::
        local button = me:dialog(npc, "요즘 성내에는 이상한 소문이 돌고 있습니다. 황실의 기보인 진황보검이 사라졌다는 소문이죠. 황실전체가 진황보검을 되찾기위해 발칵 뒤집혔다는군요.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_539_0001::
        button = me:dialog(npc, "진황보검이라면 원래 두개의 검으로 나뉘어져있으며 고대부터 중국 황실의 권력을 나타내는 보물이라 합니다. 용사님도 진황보검에 관심이 있으신가요?", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_539_0000
        end

        ::NPC_539_0002::
        local sel, list_btn = me:list(npc, "용사님께서도 진황보검을 찾아 나서겠습니까?", {
            "진황보검이라. 멋지군요!",
            "전 별로 관심이 없습니다.",
        }, true)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_539_0001
        end
        if sel == nil or sel ~= 1 then
            return
        end

        q = me:start_quest(quest.QUEST_JINHWANG)
        if q == nil then
            return
        end
        q:step(1)
        me:push_achievement(511, "진황보검을 찾아서 (북천좌상과 만나자)", 7, 20)

        me:dialog(npc, "그럼 일단 황실에 들어가 북천좌상님에게 진황보검에 대해 여쭈어보는게 좋겠네요.", false, true)
        return
    end

    if q:completed() then
        me:dialog(npc, "이미 진황보검을 찾아 황실에 바치셨잖나. 정말 대단하셨소.", false, false)
        return
    end

    local step = q:step()

    if step >= 1 and step <= 3 then
        me:dialog(npc, "그럼 일단 황실에 들어가 북천좌상님에게 진황보검에 대해 여쭈어보는게 좋겠네요.", false, true)
        return
    end

    if step == 4 then
        ::NPC_539_0004::
        local button = me:dialog(npc, "진랑검을 구하셨군요! 오.. 이것이 진황보검의 한조각이란 말인가요.. 나머지 한조각인 혈황검에 대해선 아직 아무런 소문도 듣지 못했습니다.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_539_0005::
        button = me:dialog(npc, "하지만 혈황검의 행방에 대해서 아는 분이 계신데 지금 여행중이란 얘길 들었습니다. 얼마 있으면 장안성으로 돌아오신다고 하더군요.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_539_0004
        end

        ::NPC_539_0006::
        button = me:dialog(npc, "돌아오시면 금방 찾을수 있을겁니다. 진랑검 잘 간수하시고 그때까지 기다려보심이 어떨런지..", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_539_0005
        end

        q:step(5)
        return
    end

    if step == 5 then
        me:dialog(npc, "돌아오시면 금방 찾을수 있을겁니다. 진랑검 잘 간수하시고 그때까지 기다려보심이 어떨런지..", false, true)
        return
    end

    if step == 10 then
        me:dialog(npc, "진황보검을 찾아서 황실에 돌려주셨다구요? 기어코 찾으셨군요! 정말 대단하시네요~ 찾으신것도 대단하지만 그 기보를 그냥 돌려주시다니. 정말 곧으신 분이군요.", false, true)
        me:push_achievement(511, "진황보검을 찾았다!", 7, 20)
        q:complete()
        return
    end
end