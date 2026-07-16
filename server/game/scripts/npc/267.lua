-- npc: 서쪽태극문보초
function NPC_267_CONFIRM_TEAMMATE(me, teammate_name, npc)
    local button = me:dialog(npc, string.format('%s의 동료에게 묻겠다.', teammate_name), { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end

    local selected, button = me:list(npc, string.format('너는 %s의 동료로 같이 이 문으로 들어가겠느냐?', teammate_name), { '네, 들어가겠습니다.', '아니오. 들어가지 않겠습니다.' })
    if button == DIALOG_RESULT.QUIT then
        return false
    end

    if selected == 2 then
        me:dialog(npc, '확인했다. 그럼 잠시 기다리고 있도록 하여라.', { prev = false, next = true })
        return true
    else
        return false
    end
end

function NPC_267(me, npc)
    if not me:has_items('서문열쇠', 1) then
        return
    end

    ::NPC_267_0001::
    local selected, button = me:list(npc, '열쇠를 가지고 있구나. 이 문으로 들어갈 것이냐?', { '네, 들여보내주십시오.', '아니오. 잘못왔습니다.' })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_267_0010::
    button = me:dialog(npc, '같이 온 동료가 있으면 동료와 함께 들여보내 주겠다.', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_267_0001
    end

    local teammates = {}
    while true do
        ::NPC_267_0011::
        local message = nil
        if #teammates == 0 then
            message = '동료가 있느냐?'
        else
            message = '또다른 동료가 있느냐?'
        end

        selected, button = me:list(npc, message, { '네, 있습니다.', '아니오. 없습니다.' })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if selected == 2 then
            break
        end

        local name = me:input(npc, '동료의 이름은?', { top = '동료의 이름은', bottom = '입니다.', maxlen = 12, prev = false })
        if name == DIALOG_RESULT.QUIT then
            return
        end

        local teammate = name2ch(name)
        if teammate == nil then
            me:dialog(npc, '존재하지 않는 캐릭터입니다.', { prev = false, next = true })
            goto NPC_267_0011
        end

        if me:map() ~= teammate:map() then
            me:dialog(npc, '다른 맵에 있습니다.', { prev = false, next = true })
            goto NPC_267_0011
        end

        me:dialog(npc, '동료에게 확인을 받겠다.', { prev = false, next = true })
        if teammate:script('scripts/npc/267.lua', 'NPC_267_CONFIRM_TEAMMATE', me:name(), npc) ~= true then
            me:dialog(npc, '동료에게 확인을 받지 못했습니다.', { prev = false, next = true })
            goto NPC_267_0011
        end

        teammates[#teammates + 1] = teammate
    end

    me:dialog(npc, '그럼 보내주도록 하겠다.', { prev = false, next = true })
    if not me:rmitem('서문열쇠', 1, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, '열쇠를 가지고 있지 않습니다.', { prev = false, next = true })
        return
    end

    local map = name2map('폭염귀의방')
    if map == nil then
        me:dialog(npc, '존재하지 않는 맵입니다.', { prev = false, next = true })
        return
    end

    teammates[#teammates + 1] = me
    for _, teammate in ipairs(teammates) do
        teammate:map(map, 13, 13)
    end
end