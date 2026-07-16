-- npc: 사냥도우미
local quest = require('lib.quest')
function NPC_277(me, npc)
    local q = me:quest(quest.QUEST_BEGINNER_PATH)
    if q == nil then
        me:dialog(npc, "먼저 말하기도우미에게 다녀 오세요...", { prev = false, next = false })
        return
    end

    if q:completed() then
        me:dialog(npc, "똘똘이 도우미가 다음 방에서 기다리고 있습니다.", { prev = false, next = false })
        return
    end

    local step = q:step()
    if step >= 6 then
        me:dialog(npc, "똘똘이 도우미가 다음 방에서 기다리고 있습니다.", { prev = false, next = false })
        return
    end

    if step < 3 then
        me:dialog(npc, "먼저 말하기도우미에게 다녀 오세요...", { prev = false, next = false })
        return
    end

    if step == 3 then
        ::NPC_277_0001::
        local button = me:dialog(npc, string.format("%s님, 안녕하세요?\n저는 기본적인 사냥방법을 가르쳐 드릴 사냥도우미입니다.", me:name()), { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_277_0010::
        button = me:dialog(npc, "사냥에는 일반 공격, 스킬 공격 등 여러가지 방법이 있습니다.\n하지만 지금은 가장 기본적인\n공격에 대해서 알려드리도록 하겠습니다.", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_277_0001
        end

        ::NPC_277_0011::
        button = me:dialog(npc, "공격을 하기 위해서 먼저\n몬스터 앞으로 다가가 [a]키를\n누르시거나 혹은 스페이스바를 누르시면 공격을 할 수 있습니다..", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_277_0010
        end

        ::NPC_277_0012::
        button = me:dialog(npc, "먼저 '목도'를 드릴테니 착용하시고 다람쥐와 토끼를 각각 5마리씩 사냥해 보세요.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_277_0011
        end

        if me:mkitem("목도", 1) == nil then
            me:dialog(npc, "소지품이 가득 차서 목도를 줄 수 없습니다.", { prev = false, next = false })
            return
        end
        q:step(4)
        me:dialog(npc, "모두 잡으면 '야호'라고 말해주세요.", { prev = false, next = false })
        return
    end

    local sel, btn = me:list(npc, "다람쥐와 토끼를 각각 5마리씩 사냥하셨나요?", { "네, 다 잡았어요.", "아니요, 아직이에요." }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel == 2 then
        me:dialog(npc, "토끼와 다람쥐를 각각 5마리를 사냥 해 보세요.", { prev = false, next = false })
        return
    end

    me:dialog(npc, "다람쥐와 토끼를 모두 잡으셨나요? 선물로 초보자용 갑옷을 드리겠습니다.", { prev = false, next = true })

    local armor = (me:gender() == GENDER.FEMALE) and "초보자용여자갑주" or "초보자용남자갑주"
    if me:mkitem(armor, 1) == nil then
        me:dialog(npc, "소지품이 가득 차서 갑옷을 줄 수 없습니다.", { prev = false, next = false })
        return
    end
    q:step(6)
    me:exp(me:exp() + 200)

    me:dialog(npc, "갑옷은 방어력을 낮춰주는 역할을 하며, 방어력이 낮을수록 몬스터에게 입는 데미지가 줄어들게 됩니다.", { prev = false, next = true })
    me:dialog(npc, "바람의나라에서는 다양한 갑옷이 준비되어 있으니, 모험을 통하여 더 강한 갑옷을 사용하실 수 있습니다.", { prev = false, next = true })
    me:dialog(npc, "이로서 제가 가르쳐 드릴것은 끝이 났군요. 다음 방으로 가보시길 바랍니다.", { prev = false, next = false })
end