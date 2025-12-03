function sample_group(me, npc)
    local selected = me:menu(npc, '선택', {'그룹 초대/추방', '그룹 메시지'})
    if selected == nil then
        return
    end

    if selected == 0 then
        local name = me:input(npc, '그룹 초대할 유저')
        local group = me:group()
        if group == nil then
            if me:create_group(name) then
                me:dialog(npc, '그룹이 생성되었습니다.')
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
                    me:dialog(npc, '그룹에 초대했습니다.')
                else
                    me:dialog(npc, '그룹에서 추방했습니다.')
                end
            else
                me:dialog(npc, '그룹에 초대할 수 없습니다.')
            end
        end
    elseif selected == 1 then
        local group = me:group()
        if group == nil then
            me:dialog(npc, '그룹 없음')
            return
        end

        local message = me:input(npc, '메시지')
        group = me:group()
        if group == nil then
            me:dialog(npc, '그룹 없음')
            return
        end

        group:message(message, MESSAGE_TYPE.NOTIFY)
    else
        me:chat('invalid selected')
    end
end

function sample_clan(me, npc)
    local clan = me:clan()
    if clan == nil then
        local name = me:input(npc, '생성할 문파 이름')
        local error = me:create_clan(name)
        if error ~= nil then
            me:dialog(npc, error)
        else
            me:dialog(npc, '클랜 생성 성공')
        end
    else
        local clan_name = clan:name()
        local selected = me:list(npc, string.format('클랜 이름 : %s', clan_name), {'문파 칭호 바꾸기', '문파 해체', '문파 가입', '문파 탈퇴', '문파 추방', '문파 직책 변경', '문파 메시지'})
        if selected == nil then
            return
        end

        clan = me:clan()
        if clan == nil then
            me:dialog(npc, '클랜 없음')
            return
        end

        if selected == 0 then
            local title = me:input(npc, '문파 칭호 입력', '문파 칭호는', '입니다.', 12, true)
            clan = me:clan()
            if clan == nil then
                me:dialog(npc, '클랜 없음')
                return
            end

            local error = clan:title(me, title)
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '문파 칭호 변경 성공')
            end
        elseif selected == 1 then
            local error = me:destroy_clan()
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '클랜 제거 성공')
            end

        elseif selected == 2 then
            local map = me:map()
            if map == nil then
                return
            end

            local name = me:input(npc, '상대 이름 입력')
            local nears = map:nears({me:position()}, OBJECT_TYPE.CHARACTER)
            local found = nil
            for _, ch in pairs(nears) do
                if ch:name() == name then
                    found = ch
                    break
                end
            end

            if found == nil then
                me:dialog(npc, '캐릭터 근처에 없음')
                return
            end

            local yes_or_no = found:menu(npc, string.format('%s 문파에 가입?', clan_name), {'네', '아니오'})
            if yes_or_no == 0 then
                clan = me:clan()
                if clan == nil then
                    found:dialog(npc, '클랜 없음')
                    return
                end

                local error = clan:join(me, found:name())
                if error ~= nil then
                    found:dialog(npc, error)
                else
                    found:dialog(npc, string.format('%s 문파에 가입됨', clan_name))
                    me:dialog(npc, string.format('%s가 승락함', found:name()))
                end
            else
                me:dialog(npc, string.format('%s가 거절함', found:name()))
            end
        elseif selected == 3 then
            local clan = me:clan()
            if clan == nil then
                me:dialog(npc, '클랜 없음')
                return
            end

            local error = clan:leave(me)
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '클랜 탈퇴 성공')
            end
        elseif selected == 4 then
            local name = me:input(npc, '상대 이름 입력')
            clan = me:clan()
            if clan == nil then
                me:dialog(npc, '클랜 없음')
                return
            end

            local error = clan:kick(me, name)
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '추방했음')
            end
        elseif selected == 5 then
            clan = me:clan()
            if clan == nil then
                me:dialog(npc, '클랜 없음')
                return
            end

            local name = me:input(npc, '상대 이름 입력')
            local role = me:input(npc, '직책 입력')
            local error = clan:change_role(me, name, tonumber(role))
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '직책 변경 성공')
            end
        elseif selected == 6 then
            local message = me:input(npc, '내용')
            clan = me:clan()
            if clan == nil then
                found:dialog(npc, '클랜 없음')
                return
            end

            local error = clan:message(message)
            if error ~= nil then
                me:dialog(npc, error)
            end
        else
        end
    end
end

function sample_whisper(me, npc)
    local name = me:input(npc, '귓속말할 상대 이름')
    local message = me:input(npc, '귓속말 내용')

    local error = me:whisper(name, message)
    if error ~= nil then
        me:dialog(npc, error)
    else
        me:dialog(npc, '성공')
    end
end

function sample_send_mail(me, npc)
    local name = me:input(npc, '상대 이름')
    local title = me:input(npc, '제목')
    local contents = me:input(npc, '내용')

    local error = me:send_mail(name, title, contents)
    if error ~= nil then
        me:dialog(npc, error)
    else
        me:dialog(npc, '성공')
    end
end

function sample_map(me, npc)
    local name = me:input(npc, '맵 이름')
    local error = me:map(name)
    if error ~= nil then
        me:dialog(npc, error)
    end
end

function sample_cc(me, npc)
    local selected = me:menu(npc, '선택', {'방향역전', '암흑', '채팅금지', '듣기금지'})
    if selected == nil then
        return
    end

    local current = me:cc()
    local cc = CROWD_CONTROL.NONE
    if selected == 0 then
        cc = CROWD_CONTROL.DIRECTION
    elseif selected == 1 then
        cc = CROWD_CONTROL.SIGHT
    elseif selected == 2 then
        cc = CROWD_CONTROL.CHAT
    elseif selected == 3 then
        cc = CROWD_CONTROL.HEAR
    end

    if (current & cc) == cc then
        me:cc(current & ~cc)
    else
        me:cc(current | cc)
    end
end

function sample_quest(me, npc)
    local quest = me:quest(1)
    if quest == nil then
        if not me:start_quest(1) then
            me:dialog(npc, '퀘스트 시작 실패')
        else
            local quest = me:quest(1)
            quest:param('다람쥐')
            me:dialog(npc, '퀘스트를 시작합니다. 다람쥐를 1마리 처치하세요.')
        end
        return
    end

    if quest:completed() then
        me:dialog(npc, '이미 퀘스트를 완료하셨습니다.')
        return
    end

    if quest:step() == 0 then
        if quest:completed_progress() then
            if not quest:inc_step() then
                me:dialog(npc, '퀘스트 단계 증가 실패')
            else
                quest:param('토끼')
                me:dialog(npc, '잘 하셨습니다. 다음은 토끼를 2마리 잡으세요')
            end
        else
            local progress = quest:progress()
            local max_progress = quest:model():progress()
            me:dialog(npc, string.format('다람쥐 처치 %d/%d', progress, max_progress))
        end
        return
    end

    if quest:step() == 1 then
        if quest:completed_progress() then
            if not quest:inc_step() then
                me:dialog(npc, '퀘스트 단계 증가 실패')
            else
                quest:param('삽사리')
                me:dialog(npc, '잘 하셨습니다. 다음은 삽사리를 3마리 잡으세요')
            end
        else
            local progress = quest:progress()
            local max_progress = quest:model():progress()
            me:dialog(npc, string.format('토끼 처치 %d/%d', progress, max_progress))
        end
        return
    end

    if quest:step() == 2 then
        if quest:completed_progress() then
            if not quest:complete() then
                me:dialog(npc, '퀘스트 완료 실패. 인벤토리나 금전을 확인하세요.')
            else
                me:dialog(npc, '퀘스트를 완료했습니다. 보상을 드렸습니다.')
            end
        else
            local progress = quest:progress()
            local max_progress = quest:model():progress()
            me:dialog(npc, string.format('삽사리 처치 %d/%d', progress, max_progress))
        end
        return
    end
end

function NPC_0(me, npc)
    local selected = me:menu(npc, '안녕하세요. 무엇을 도와드릴까요?', {'group','clan','whisper','send_mail','map','cc','quest'})
    if selected == nil then
        return
    end

    if selected == 0 then
        return sample_group(me, npc)
    end
    if selected == 1 then
        return sample_clan(me, npc)
    end
    if selected == 2 then
        return sample_whisper(me, npc)
    end
    if selected == 3 then
        return sample_send_mail(me, npc)
    end
    if selected == 4 then
        return sample_map(me, npc)
    end
    if selected == 5 then
        return sample_cc(me, npc)
    end
    if selected == 6 then
        return sample_quest(me, npc)
    end
end