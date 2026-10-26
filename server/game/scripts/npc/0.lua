-- npc: 낙랑
local quest = require('lib.quest')
local npc_lib = require('lib.npc')

local function main(me, npc)
    npc_lib.nakrang_intro(me, npc)
end

local function sample_group(me, npc)
    local selected = me:menu(npc, '선택', { '그룹 초대/추방', '그룹 메시지' })
    if selected == nil then
        return
    end

    if selected == 1 then
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

            local error = group:toggle(me, name)
            if error == nil then
                if found == nil then
                    me:dialog(npc, '그룹에 초대했습니다.')
                else
                    me:dialog(npc, '그룹에서 추방했습니다.')
                end
            else
                me:dialog(npc, '그룹에 초대할 수 없습니다.')
            end
        end
    elseif selected == 2 then
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

local function sample_clan(me, npc)
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
        ::NPC_0_0001::
        local selected, list_btn = me:list(npc, string.format('클랜 이름 : %s', clan_name),
            { '문파 칭호 바꾸기', '문파 해체', '문파 가입', '문파 탈퇴', '문파 추방', '문파 직책 변경', '문파 메시지' }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if selected == nil then
            return
        end

        clan = me:clan()
        if clan == nil then
            me:dialog(npc, '클랜 없음')
            return
        end

        if selected == 1 then
            local title = me:input(npc, '문파 칭호 입력', { top = '문파 칭호는', bottom = '입니다.', maxlen = 12, prev = true })
            if title == DIALOG_RESULT.QUIT then
                return
            end
            if title == DIALOG_RESULT.PREV then
                goto NPC_0_0001
            end
            if type(title) ~= 'string' or title == '' then
                return
            end
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
        elseif selected == 2 then
            local error = me:destroy_clan()
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '클랜 제거 성공')
            end

        elseif selected == 3 then
            local name = me:input(npc, '상대 이름 입력')
            local found = nil
            for _, ch in pairs(me:nears(OBJECT_TYPE.CHARACTER)) do
                if ch:name() == name then
                    found = ch
                    break
                end
            end

            if found == nil then
                me:dialog(npc, '캐릭터 근처에 없음')
                return
            end

            local yes_or_no = found:menu(npc, string.format('%s 문파에 가입?', clan_name), { '네', '아니오' })
            if yes_or_no == 1 then
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
        elseif selected == 4 then
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
        elseif selected == 5 then
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
        elseif selected == 6 then
            clan = me:clan()
            if clan == nil then
                me:dialog(npc, '클랜 없음')
                return
            end

            local name = me:input(npc, '상대 이름 입력')
            local role = tonumber(me:input(npc, '직책 입력'))
            if role == nil then
                me:dialog(npc, '직책은 숫자로 입력')
                return
            end
            local error = clan:change_role(me, name, role)
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '직책 변경 성공')
            end
        elseif selected == 7 then
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

local function sample_whisper(me, npc)
    local name = me:input(npc, '귓속말할 상대 이름')
    local message = me:input(npc, '귓속말 내용')

    local error = me:whisper(name, message)
    if error ~= nil then
        me:dialog(npc, error)
    else
        me:dialog(npc, '성공')
    end
end

local function sample_send_mail(me, npc)
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

local function sample_map(me, npc)
    local name = me:input(npc, '맵 이름')
    local error = me:map(name)
    if error ~= nil then
        me:dialog(npc, error)
    end
end

local function sample_cc(me, npc)
    local selected = me:menu(npc, '선택', { '방향역전', '암흑', '채팅금지', '듣기금지' })
    if selected == nil then
        return
    end

    local current = me:cc()
    local cc = CROWD_CONTROL.NONE
    if selected == 1 then
        cc = CROWD_CONTROL.DIRECTION
    elseif selected == 2 then
        cc = CROWD_CONTROL.SIGHT
    elseif selected == 3 then
        cc = CROWD_CONTROL.CHAT
    elseif selected == 4 then
        cc = CROWD_CONTROL.HEAR
    end

    if (current & cc) == cc then
        me:cc(current & ~cc)
    else
        me:cc(current | cc)
    end
end

local function sample_quest(me, npc)
    local q = me:quest(quest.QUEST_NAKRANG_HUNT)
    if q == nil then
        if me:level() < 10 then
            me:dialog(npc, '레벨이 부족합니다.')
            return
        end

        q = me:start_quest(quest.QUEST_NAKRANG_HUNT)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패')
            return
        end
        q:param('다람쥐')
        me:dialog(npc, '퀘스트를 시작합니다. 다람쥐를 1마리 처치하세요.')
        return
    end

    if q:completed() then
        me:dialog(npc, '이미 퀘스트를 완료하셨습니다.')
        return
    end

    local step = q:step()
    if step == 0 then
        local progress = q:progress()
        local goal = 1
        if progress >= goal then
            q:inc_step(1)
            q:param('토끼')
            me:dialog(npc, '잘 하셨습니다. 다음은 토끼를 2마리 잡으세요')
        else
            me:dialog(npc, string.format('다람쥐 처치 %d/%d', progress, goal))
        end
    elseif step == 1 then
        local progress = q:progress()
        local goal = 2
        if progress >= goal then
            q:inc_step(1)
            q:param('삽사리')
            me:dialog(npc, '잘 하셨습니다. 다음은 삽사리를 3마리 잡으세요')
        else
            me:dialog(npc, string.format('토끼 처치 %d/%d', progress, goal))
        end
    elseif step == 2 then
        local progress = q:progress()
        local goal = 3
        if progress >= goal then
            if not me:reward('quest1.step.3') then
                me:dialog(npc, '퀘스트 보상 부여 실패')
                return
            end

            q:complete()
            me:dialog(npc, '퀘스트를 완료했습니다. 보상을 드렸습니다.')
        else
            me:dialog(npc, string.format('삽사리 처치 %d/%d', progress, goal))
        end
    end
end

local function sample_menu(me, npc)
    local selected = me:menu(npc, '안녕하세요. 무엇을 도와드릴까요?', { 'group', 'clan', 'whisper', 'send_mail', 'map', 'cc', 'q' })
    if selected == nil then
        return
    end
    if selected == 1 then
        return sample_group(me, npc)
    end
    if selected == 2 then
        return sample_clan(me, npc)
    end
    if selected == 3 then
        return sample_whisper(me, npc)
    end
    if selected == 4 then
        return sample_send_mail(me, npc)
    end
    if selected == 5 then
        return sample_map(me, npc)
    end
    if selected == 6 then
        return sample_cc(me, npc)
    end
    if selected == 7 then
        return sample_quest(me, npc)
    end
end

return {
    on_click = function(me, npc)
        main(me, npc)
    end,

    on_birthday = function(me, npc)
        local button = nil
        ::NPC_0_0002::
        button = me:dialog(npc, '대단히 중요하니 끝까지 읽어주세요! 빈번히 발생하는 아이디 해킹을 미연에 방지하기 위해 또 하나의 2차 비밀번호를 정해야 합니다.',
            { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_0_0003::
        button = me:dialog(npc, '이 2차 비밀번호는 한번 결정을 하시면 변경이 불가능 합니다. 이 2차 비밀번호는 자신의 게임 접속 비밀번호를 바꿀 때에만 사용합니다.',
            { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_0_0002
        end

        ::NPC_0_0004::
        button = me:dialog(npc, '이 2차 비밀번호는 자신의 생년월일로 해주세요. 예를 들면 자신의 생일이 92년 04월 17일이면, 920417으로 입력해주세요. (반드시 6자리)',
            { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_0_0003
        end

        ::NPC_0_0005::
        button = me:dialog(npc, '이 2차 비밀번호는 분실하실 이유가 없기 때문에(자신의 생년월일이므로) 이 2차 비밀번호를 모르시는 분은 해당 아이디에 대한 소유권을 박탈합니다.',
            { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_0_0004
        end

        ::NPC_0_0006::
        button = me:dialog(npc, '이 2차 비밀번호를 모르시고 처음의 비밀번호 분실신고를 하셔도 저희는 가르쳐 드릴수가 없습니다. 그 만큼 중요한 것이오니 신중히 그리고 정확히 입력해주세요.',
            { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_0_0005
        end

        ::NPC_0_0007::
        button = me:dialog(npc, '또한 이 두번째 비밀번호(생년월일)를 입력하지 않으시고 다음에 비밀번호 분실신고를 하셔도 저희는 해드릴수가 없으니 반드시 두번째 비번을 입력하시기 바랍니다.',
            { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_0_0006
        end

        ::NPC_0_0008::
        local birthday = me:input(npc, '자신의 생년월일을 입력해 주세요.',
            { top = '저의 생년월일은', bottom = '입니다.', maxlen = 6, prev = true })
        if birthday == DIALOG_RESULT.QUIT then
            goto NPC_0_0008
        end
        if birthday == DIALOG_RESULT.PREV then
            goto NPC_0_0007
        end

        if string.match(birthday, '^%d%d%d%d%d%d$') == nil or birthday == '000000' then
            button = me:dialog(npc, '두번째 비밀번호를 정하지 않으면 게임을 할 수 없습니다. 반드시 해주세요.', { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            goto NPC_0_0008
        end

        local sel, list_btn = me:list(npc, string.format('당신의 생년월일이 %s가 맞습니까?', birthday), { '예', '아니오' },
            { prev = true })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_0_0008
        end

        if sel == 1 then
            me:birthday(tonumber(birthday))
            return
        end

        goto NPC_0_0008
    end
}
