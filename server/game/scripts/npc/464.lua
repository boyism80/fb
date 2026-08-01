-- npc: 소원왕
local quest = require('lib.quest')
local function do_sub2_start(me, npc)
    ::NPC_464_0001::
    local b = me:dialog(npc, "경계하실 것 없다. 상의하고 싶은 문제가 있는 것일 뿐이니까. 나를 도와주고 싶은건가.", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_464_0010::
    b = me:dialog(npc, "그러면, 이 편지를 토원왕에게 전해주셔야한다. 가장 지혜로운 부족의 우두머리이시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        goto NPC_464_0001
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_464_0011::
    b = me:dialog(npc, "상의하고 싶은 문제가 있으시니, 일단 편지를 전해주시기만 하시면, 나머지는 토원왕이 해결해 주실거다.", { prev = false, next = true })
    if b == DIALOG_RESULT.PREV then
        goto NPC_464_0010
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    me:mkitem("원숭이편지", 1)
    local q = me:quest(quest.QUEST_SKULL_NECKLACE_2)
    if q == nil then
        q = me:start_quest(quest.QUEST_SKULL_NECKLACE_2)
        if q == nil then
            return true
        end
    end
    if q then
        q:step(1)
    end
    return true
end

local function do_sub2_complete(me, npc)
    ::NPC_464_0020::
    local b = me:dialog(npc, "고맙다, 이제 다른 두목들과의 연락도 조금은 원활해지실거다. 이건 병에 걸린 우리 아이들과 싸우시던", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_464_0021::
    b = me:dialog(npc, "인간들이 버리시고 가신 물건 중에서 제일 무거우시고 쓸만하신 무기시다. 요긴하게 쓰셔야 한다.", { prev = false, next = true })
    if b == DIALOG_RESULT.PREV then
        goto NPC_464_0020
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    me:mkitem("비철단도", 1)
    local q = me:quest(quest.QUEST_SKULL_NECKLACE_2)
    if q then
        q:step(3)
    end
    return true
end

return {
    on_click = function(me, npc)
        local q2 = me:quest(quest.QUEST_SKULL_NECKLACE_2)

        if q2 == nil or q2:step() == 0 then
            if do_sub2_start(me, npc) then
                return
            end
            return
        end

        if q2:step() == 1 then
            me:dialog(npc, "상의하고 싶은 문제가 있으시니, 일단 편지를 전해주시기만 하시면, 나머지는 토원왕이 해결해 주실거다.", { prev = false, next = false })
            return
        end

        if q2:step() == 2 then
            if do_sub2_complete(me, npc) then
                return
            end
            return
        end

        local main_q = me:quest(quest.QUEST_SKULL_NECKLACE)
        if main_q and main_q:step() == 33 then
            if not me:has_items("마른갈대", 1) then
                me:dialog(npc, "마른갈대를 구해서 왕들에게 하나씩 나누어 주게.", { prev = false, next = false })
                return
            end
            local b = me:dialog(npc, "마른 갈대를 나눠주고 있다고 들었네. 수고하는 모습이 참 보기 좋군. 더 수고해주게.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            if not me:rmitem("마른갈대", 1, ITEM_DELETE_TYPE.GIVE) then
                me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                return
            end
            main_q:step(34)
            return
        end

        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
    end
}
