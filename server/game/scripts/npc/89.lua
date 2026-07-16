-- npc: 문파대리인
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
        local selected = me:list(npc, string.format('클랜 이름 : %s', clan_name), {'문파 칭호 바꾸기', '문파 해체', '문파 가입', '문파 탈퇴', '문파 추방', '문파 직책 변경', '문파 메시지'})
        if selected == nil then
            return
        end

        clan = me:clan()
        if clan == nil then
            me:dialog(npc, '클랜 없음')
            return
        end

        if selected == 1 then
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
        elseif selected == 2 then
            local error = me:destroy_clan()
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '클랜 제거 성공')
            end

        elseif selected == 3 then
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
            local role = me:input(npc, '직책 입력')
            local error = clan:change_role(me, name, tonumber(role))
            if error ~= nil then
                me:dialog(npc, error)
            else
                me:dialog(npc, '직책 변경 성공')
            end
        elseif selected == 7 then
            local message = me:input(npc, '내용')
            clan = me:clan()
            if clan == nil then
                me:dialog(npc, '클랜 없음')
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

function NPC_89(me, npc)
    sample_clan(me, npc)
end