
function NPC_543(me, npc)
    local quest = me:quest(QUEST_JINHWANG)
    if quest == nil or quest:step() ~= 1 then
        me:dialog(npc, "....", false, true)
        return
    end

    ::NPC_543_0000::
    local button = me:dialog(npc, "보아하니 진황보검 때문에 갇힌거 같은데. 요즘 황제전하가 진황보검때문에 신경이 날카로워 지셔서 그얘기만 나오면 모두 하옥시켜버린다지.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_543_0001::
    button = me:dialog(npc, "이건 비밀인데 지난번에 황제전하가 옥에 오셨을때 들은게 있다네. 진황보검의 한자루는 어딨는지 알아냈다고. 흉노족의 장로가 지니고 있다고 하더군.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_543_0000
    end

    ::NPC_543_0002::
    button = me:dialog(npc, "그런데 그 자올의 아들이 천상복숭아가 없으면 생명이 위험하다고 하네. 천상복숭아를 가져가면 협상을 해볼수 있지 않을까?", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_543_0001
    end

    ::NPC_543_0003::
    button = me:dialog(npc, "자네에게선 뭔가 범상치 않은 기운이 느껴지는군. 나는 보잘것없는 간수지만 자네를 도와주고 싶군. 내 절친한 친구하나가 부여성 북문을 지키고 있는데 그가 아마 천상복숭아를 가지고있을거야.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_543_0002
    end

    ::NPC_543_0004::
    button = me:dialog(npc, "이 종이쪽지를 가져가면 내가 보냈다는걸 알아볼걸세. 자네가 나갈 수 있을때의 얘기지만 말이야..", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if me:mkitem("종이쪽지", 1) == nil then
        me:dialog(npc, "소지품 칸을 2칸 이상 비워주세요.", false, false)
        return
    end

    quest:step(2)
end
