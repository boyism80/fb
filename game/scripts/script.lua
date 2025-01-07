function group_lock(me, group)
    if group == nil then
        me:chat('그룹이 없어용')
        return false
    end

    local map = me:map()
    if map == nil then
        return false
    end

    local nears = group:nears(me:map(), {me:position()})
    for _, near in pairs(nears) do
        near:chat(near:name())
    end
    return true, #nears
end

function sample_message()
    broadcast('message NOTIFY', MESSAGE_TYPE_NOTIFY)
    broadcast('message BLUE', MESSAGE_TYPE_BLUE)
    broadcast('message STATE', MESSAGE_TYPE_STATE)
    broadcast('message SHOUT', MESSAGE_TYPE_SHOUT)
    broadcast('message WORLD', MESSAGE_TYPE_WORLD)
    broadcast('message POPUP', MESSAGE_TYPE_POPUP)
    broadcast('message YELLOW', MESSAGE_TYPE_YELLOW)
    broadcast('message BROWN', MESSAGE_TYPE_BROWN)
end

function sample_group(me)
    local npc = name2npc('낙랑')
    local name = npc:input(me, '그룹 초대할 유저')
    local selected = npc:menu(me, '선택', {'그룹 액션', '그룹 메시지'})
    if selected == nil then
        return
    end

    if selected == 1 then
        local group = me:group()
        if group == nil then
            if me:create_group(name) then
                npc:dialog(me, '그룹이 생성되었습니다.')
            end
        else
            local found = nil
            for _, member in pairs(group:members()) do
                if member == name then
                    found = member
                    break
                end
            end

            if me:create_group(name) then
                if found == nil then
                    npc:dialog(me, '그룹에 초대했습니다.')
                else
                    npc:dialog(me, '그룹에서 추방했습니다.')
                end
            else
                npc:dialog(me, '그룹에 초대할 수 없습니다.')
            end
        end
    elseif selected == 2 then
        local group = me:group()
        if group == nil then
            npc:dialog(me, '그룹 없음')
            return
        end

        local message = npc:input(me, '메시지')
        group = me:group()
        if group == nil then
            npc:dialog(me, '그룹 없음')
            return
        end

        group:messagee(message, MESSAGE_TYPE_NOTIFY)
    end
end

function sample_clan(me)
    local npc = name2npc('낙랑')
    local clan = me:clan()
    if clan == nil then
        local name = npc:input(me, '생성할 문파 이름')
        local error = me:create_clan(name)
        if error ~= nil then
            npc:dialog(me, error)
        else
            npc:dialog(me, '클랜 생성 성공')
        end
    else
        local clan_name = clan:name()
        local selected = npc:menu(me, string.format('클랜 이름 : %s', clan_name), {'문파 칭호 바꾸기', '문파 해체', '문파 가입', '문파 추방', '메시지'})
        if selected == nil then
            return
        end

        clan = me:clan()
        if clan == nil then
            npc:dialog(me, '클랜 없음')
            return
        end

        if selected == 0 then
            local title = npc:input(me, '문파 칭호 입력')
            clan = me:clan()
            if clan == nil then
                npc:dialog(me, '클랜 없음')
                return
            end

            local error = clan:title(title)
            if error ~= nil then
                npc:dialog(me, error)
            else
                npc:dialog(me, '문파 칭호 변경 성공')
            end
        elseif selected == 1 then
            local error = me:destroy_clan()
            if error ~= nil then
                npc:dialog(me, error)
            else
                npc:dialog(me, '클랜 제거 성공')
            end

        elseif selected == 2 then
            local map = me:map()
            if map == nil then
                return
            end

            local name = npc:input(me, '상대 이름 입력')
            local nears = map:nears({me:position()}, OBJECT_TYPE_CHARACTER)
            local found = nil
            for _, ch in pairs(nears) do
                if ch:name() == name then
                    found = ch
                    break
                end
            end

            if found == nil then
                npc:dialog(me, '캐릭터 근처에 없음')
                return
            end

            me:switch_context(found)
            local yes_or_no = npc:menu(found, string.format('%s 문파에 가입?', clan_name), {'네', '아니오'})
            if yes_or_no == 0 then
                clan = me:clan()
                if clan == nil then
                    npc:dialog(found, '클랜 없음')
                    return
                end

                local error = clan:join(found)
                if error ~= nil then
                    npc:dialog(found, error)
                else
                    npc:dialog(found, string.format('%s 문파에 가입됨', clan_name))

                    found:switch_context(me)
                    npc:dialog(me, string.format('%s가 승락함', found:name()))
                end
            else
                npc:dialog(me, string.format('%s가 거절함', found:name()))
            end
        elseif selected == 3 then
            local name = npc:input(me, '상대 이름 입력')
            clan = me:clan()
            if clan == nil then
                npc:dialog(found, '클랜 없음')
                return
            end

            local error = clan:leave(name, true)
            if error ~= nil then
                npc:dialog(me, error)
            else
                npc:dialog(me, '추방했음')
            end
        elseif selected == 4 then
            local message = npc:input(me, '내용')
            clan = me:clan()
            if clan == nil then
                npc:dialog(found, '클랜 없음')
                return
            end

            local error = clan:message(message)
            if error ~= nil then
                npc:dialog(me, error)
            end
        else
        end
    end
end

function sample_whisper(me)
    local npc = name2npc('낙랑')
    local name = npc:input(me, '귓속말할 상대 이름')
    local message = npc:input(me, '귓속말 내용')

    local error = me:whisper(name, message)
    if error ~= nil then
        npc:dialog(me, error)
    else
        npc:dialog(me, '성공')
    end
end

function sample_send_mail(me)
    local npc = name2npc('낙랑')
    local name = npc:input(me, '상대 이름')
    local title = npc:input(me, '제목')
    local contents = npc:input(me, '내용')

    local error = me:send_mail(name, title, contents)
    if error ~= nil then
        npc:dialog(me, error)
    else
        npc:dialog(me, '성공')
    end
end

function sample_map(me)
    local npc = name2npc('낙랑')
    local name = npc:input(me, '맵 이름')
    local error = me:map(name)
    if error ~= nil then
        npc:dialog(me, error)
    end
end

function samplee_cc(me)
    local npc = name2npc('낙랑')
    local selected = npc:menu(me, '선택', {'방향역전', '암흑', '채팅금지', '듣기금지'})
    if selected == nil then
        return
    end

    local current = me:cc()
    local cc = CROWD_CONTROL_NONE
    if selected == 0 then
        cc = CROWD_CONTROL_DIRECTION
    elseif selected == 1 then
        cc = CROWD_CONTROL_SIGHT
    elseif selected == 2 then
        cc = CROWD_CONTROL_CHAT
    elseif selected == 3 then
        cc = CROWD_CONTROL_HEAR
    end

    if (current & cc) == cc then
        me:cc(current & ~cc)
    else
        me:cc(current | cc)
    end
end

function func(me)

    samplee_cc(me)

    -- local success, size = me:group(group_lock)
    -- local npc = name2npc('낙랑')
    -- npc:dialog(me, string.format('%s, %s', success, size))
    
    -- local items = me:items()

    -- for slot, item in pairs(items) do
    --     local model = item:model()
    --     if model:attr(ITEM_ATTR_EQUIPMENT) and model:repair_price() ~= nil then
    --         local current = item:durability()
    --         local max = model:durability()
    --         item:durability(max - 1)
    --     end
    -- end

    -- npc:repair(me)

    -- local map = me:map()
    -- if map == nil then
    --     return
    -- end

    -- local nears = map:nears({me:position()})
    -- for _, near in pairs(nears) do
    --     near:chat(near:name())
    -- end
end