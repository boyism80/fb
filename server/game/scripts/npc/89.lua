-- npc: 문파대리인
local function find_nearby_character(me, name)
    local map = me:map()
    if map == nil then
        return nil
    end

    local nears = map:nears({me:position()}, OBJECT_TYPE.CHARACTER)
    for _, ch in pairs(nears) do
        if ch:name() == name then
            return ch
        end
    end

    return nil
end

local function clan_master_name(clan)
    for _, member in pairs(clan:members()) do
        if member:role() == CLAN_ROLE.MASTER then
            return member:name()
        end
    end

    return nil
end

local function find_target_clan(me, npc, prompt)
    local target_name = me:input(npc, prompt)
    local found = find_nearby_character(me, target_name)
    if found == nil then
        me:dialog(npc, '외교를 수행하기 위해서는, 상대 문파의 문파장과 함께 와야 하오.')
        return nil, nil
    end

    local target_clan = found:clan()
    if target_clan == nil then
        me:dialog(npc, '해당 인물은 문파가 없구려.')
        return nil, nil
    end

    if clan_master_name(target_clan) ~= found:name() then
        me:dialog(npc, '외교를 수행하기 위해서는, 상대 문파의 문파장과 함께 와야 하오.')
        return nil, nil
    end

    return found, target_clan
end

local function request_diplomacy(me, npc, found, target_clan, wait_message, confirm_message, success_message, fail_message, action)
    me:dialog(npc, wait_message, { immediate = true })

    local yes_or_no = found:menu(npc, confirm_message, {'네', '아니오'})
    if yes_or_no ~= 1 then
        me:dialog(npc, fail_message)
        return
    end

    local clan = me:clan()
    if clan == nil then
        found:dialog(npc, '문파가 없구려.')
        return
    end

    local error = action(clan, target_clan)
    if error ~= nil then
        me:dialog(npc, error)
    else
        found:dialog(npc, success_message, { immediate = true })
        me:dialog(npc, success_message, { immediate = true })
    end
end

local function clan_registration(me, npc)
    local yes_or_no = me:list(npc, '당신이 문파를 만들고 싶다면 잘 찾아오셨소.\n문파를 세울 준비가 되셨소?', {'예, 준비됐습니다.', '그런거 없습니다. 포기할게요 ㅜ.ㅜ'})
    if yes_or_no ~= 1 then
        me:dialog(npc, '하하! 하나의 문파를 이끈다는 것이 그리 쉬운 일인 줄 알았소? 좀 더 준비를 하고 오시오.')
        return
    end

    local name = me:input(npc, '그래, 문파명은 무엇으로 하시려우?')
    local error = me:create_clan(name)
    if error ~= nil then
        me:dialog(npc, error)
    else
        me:dialog(npc, '축하하오! 이제 그대는 문파의 한 문주가 되었소. 문원을 잘 이끌어갈 수 있는 좋은 문주가 되길 바라겠소!')
    end
end

local function clan_info(me, npc, clan)
    local selected = me:list(npc, '어떤 정보를 원하십니까?', {'성문파 현황', '문파 구성원', '동맹 현황', '전쟁 현황'})
    if selected == nil then
        return
    end

    if selected == 1 then
        me:dialog(npc, '아직 준비 중인 기능입니다.')
    elseif selected == 2 then
        local title = clan:title()
        local members = clan:members()
        local master_name = clan_master_name(clan)

        local lines = {
            string.format('문파명 : %s', clan:name()),
            string.format('칭호 : %s', title ~= nil and title or '없음'),
            string.format('문파장 : %s', master_name ~= nil and master_name or '알 수 없음'),
            string.format('인원 : %d명', #members),
        }

        for _, member in pairs(members) do
            table.insert(lines, string.format(' - %s', member:name()))
        end

        me:dialog(npc, table.concat(lines, '\n'))
    elseif selected == 3 then
        local allied_id = clan:allied_clan_id()
        if allied_id == nil then
            me:dialog(npc, '동맹현황: \n없음')
        else
            local allied_clan = id2clan(allied_id)
            local allied_name = allied_clan ~= nil and allied_clan:name() or tostring(allied_id)
            me:dialog(npc, string.format('동맹현황: \n%s', allied_name))
        end
    elseif selected == 4 then
        local enemy_ids = clan:enemy_clan_ids()
        if #enemy_ids == 0 then
            me:dialog(npc, '전쟁현황: \n없음')
        else
            local names = {}
            for _, enemy_id in pairs(enemy_ids) do
                local enemy_clan = id2clan(enemy_id)
                table.insert(names, enemy_clan ~= nil and enemy_clan:name() or tostring(enemy_id))
            end
            me:dialog(npc, string.format('전쟁현황: \n%s', table.concat(names, '\n')))
        end
    end
end

local function clan_operation(me, npc, clan)
    local selected = me:list(npc, '어떤 문파 운영을 하시겠습니까?', {'문파칭호 부여', '문원 등용', '문원 축출', '직책 변경', '문파 공지', '문파 해체'})
    if selected == nil then
        return
    end

    clan = me:clan()
    if clan == nil then
        me:dialog(npc, '문파가 없구려.')
        return
    end

    if selected == 1 then
        local title = me:input(npc, '문파에 새로 내걸 칭호는 무엇으로 하시려우?')
        clan = me:clan()
        if clan == nil then
            me:dialog(npc, '문파가 없구려.')
            return
        end

        local error = clan:title(me, title)
        if error ~= nil then
            me:dialog(npc, error)
        else
            me:dialog(npc, '문파의 칭호를 새로 내걸었소.')
        end
    elseif selected == 2 then
        local name = me:input(npc, '등용하고자 하는 회원의 이름은 무엇이오?')
        local found = find_nearby_character(me, name)
        if found == nil then
            me:dialog(npc, '등용하려는 회원이 이 자리에 보이지 않아 받을 수 없소.')
            return
        end

        me:dialog(npc, string.format('%s에게 등용 요청서를 보냈으니, 잠시만 기다려보시오.', found:name()), { immediate = true })

        local yes_or_no = found:menu(npc, string.format('%s 문파에서 문파가입 요청이 들어왔습니다. 가입하시겠습니까? ', clan:name()), {'네, 가입하겠습니다.', '아니오, 안할래요.'})
        if yes_or_no == 1 then
            clan = me:clan()
            if clan == nil then
                found:dialog(npc, '문파가 없구려.')
                return
            end

            local error = clan:join(me, found:name())
            if error ~= nil then
                found:dialog(npc, error)
            else
                found:dialog(npc, string.format('%s 문파에 가입되었습니다.', clan:name()), { immediate = true })
                me:dialog(npc, string.format('%s의 가입 수속이 처리되었소.', found:name()), { immediate = true })
            end
        else
            me:dialog(npc, string.format('%s가 거절했습니다.', found:name()))
        end
    elseif selected == 3 then
        local name = me:input(npc, '축출하고자 하는 회원의 이름은 무엇이오?')
        clan = me:clan()
        if clan == nil then
            me:dialog(npc, '문파가 없구려.')
            return
        end

        local error = clan:kick(me, name)
        if error ~= nil then
            me:dialog(npc, error)
        else
            me:dialog(npc, string.format('%s의 축출 수속이 처리되었소.', name))
        end
    elseif selected == 4 then
        local name = me:input(npc, '직책을 바꾸고자 하는 회원의 이름은 무엇이오?')
        local role = me:input(npc, '새로 임명할 직책 번호를 입력하시오.')
        local error = clan:change_role(me, name, tonumber(role))
        if error ~= nil then
            me:dialog(npc, error)
        else
            me:dialog(npc, string.format('%s의 직책 임명 수속이 처리되었소.', name))
        end
    elseif selected == 5 then
        local message = me:input(npc, '문원들에게 알릴 내용을 적어주시오.')
        clan = me:clan()
        if clan == nil then
            me:dialog(npc, '문파가 없구려.')
            return
        end

        local error = clan:message(message)
        if error ~= nil then
            me:dialog(npc, error)
        end
    elseif selected == 6 then
        local yes_or_no = me:list(npc, string.format('%s 문파를 정말로 해체하시겠소? 되돌릴 수 없소.', clan:name()), {'예, 해체하겠습니다.', '아니오'})
        if yes_or_no ~= 1 then
            return
        end

        local error = me:destroy_clan()
        if error ~= nil then
            me:dialog(npc, error)
        else
            me:dialog(npc, '문파 해체 수속이 처리되었소.')
        end
    end
end

local function clan_diplomacy(me, npc, clan)
    local selected = me:list(npc, '행하실 외교를 선택하십시요.', {'동맹 신청 ( 同盟 )', '동맹 파기 ( 同盟破棄 )', '선전포고 ( 宣戰布告 )', '종전 ( 終戰 )'})
    if selected == nil then
        return
    end

    clan = me:clan()
    if clan == nil then
        me:dialog(npc, '문파가 없구려.')
        return
    end

    local clan_name = clan:name()

    if selected == 1 then
        if clan:allied_clan_id() ~= nil then
            me:dialog(npc, '동맹 문파를 \n1개이상 보유할 수\n 없습니다.')
            return
        end

        local found, target_clan = find_target_clan(me, npc, '동맹 선언을 하실 상대 문파장의 이름을 말씀해 주시오.\n상대 문파의 문파장과 함께 와야 한다오.')
        if found == nil then
            return
        end

        request_diplomacy(me, npc, found, target_clan,
            string.format('%s 문파의 %s에게 동맹 요청서를 보냈으니, 잠시만 기다려보시오.', target_clan:name(), found:name()),
            string.format('%s문파에서 동맹문파 요청이 들어왔습니다. 동맹을 맺으시겠습니까? ', clan_name),
            string.format('<%s> 문파와 <%s> 문파의 동맹이 성공적으로 체결되었습니다.', clan_name, target_clan:name()),
            string.format('<%s> 문파와 <%s> 문파의 동맹이 체결되지 못했습니다.', clan_name, target_clan:name()),
            function(c, target) return c:request_ally(me, target) end)
    elseif selected == 2 then
        local found, target_clan = find_target_clan(me, npc, '동맹파기 선언을 하실 상대 문파장의 이름을 말씀해 주시오.\n상대 문파의 문파장과 함께 와야 한다오.')
        if found == nil then
            return
        end

        request_diplomacy(me, npc, found, target_clan,
            string.format('%s 문파의 %s에게 동맹 파기 요청서를 보냈으니, 잠시만 기다려보시오.', target_clan:name(), found:name()),
            string.format('%s문파에서 동맹파기 요청이 들어왔습니다. 동맹을 파기 하시겠습니까? ', clan_name),
            string.format('<%s> 문파와 <%s> 문파의 동맹파기가 완료되었습니다.', clan_name, target_clan:name()),
            string.format('<%s> 문파와 <%s> 문파의 동맹파기가 완료되지 못했습니다.', clan_name, target_clan:name()),
            function(c, target) return c:break_ally(me) end)
    elseif selected == 3 then
        local found, target_clan = find_target_clan(me, npc, '선전포고 선언을 하실 상대 문파장의 이름을 말씀해 주시오.\n상대 문파의 문파장과 함께 와야 한다오.')
        if found == nil then
            return
        end

        request_diplomacy(me, npc, found, target_clan,
            string.format('%s 문파의 %s에게 선전포고 요청서를 보냈으니, 잠시만 기다려보시오.', target_clan:name(), found:name()),
            string.format('%s문파에서 선전포고 요청이 들어왔습니다. 정말로 전쟁을 시작하시겠습니까? ', clan_name),
            string.format('<%s> 문파와 <%s> 문파의 적대관계 설정이 완료되었습니다.', clan_name, target_clan:name()),
            string.format('<%s> 문파와 <%s> 문파의 적대관계 설정이 완료되지 못했습니다.', clan_name, target_clan:name()),
            function(c, target) return c:declare_enemy(me, target) end)
    elseif selected == 4 then
        local found, target_clan = find_target_clan(me, npc, '종전 협약을 하실 상대 문파장의 이름을 말씀해 주시오.\n상대 문파의 문파장과 함께 와야 한다오.')
        if found == nil then
            return
        end

        request_diplomacy(me, npc, found, target_clan,
            string.format('%s 문파의 %s에게 종전 협약 요청서를 보냈으니, 잠시만 기다려보시오.', target_clan:name(), found:name()),
            string.format('%s문파에서 종전 협약 요청이 들어왔습니다. 종전 하시겠습니까? ', clan_name),
            string.format('<%s> 문파와 <%s> 문파의 종전 협약이 완료되었습니다.', clan_name, target_clan:name()),
            string.format('<%s> 문파와 <%s> 문파의 종전 협약이 완료되지 못했습니다.', clan_name, target_clan:name()),
            function(c, target) return c:end_enemy(me, target) end)
    end
end

local function clan_leave(me, npc, clan)
    local yes_or_no = me:list(npc, '문파를 탈퇴하시겠소?', {'예, 탈퇴시켜 주십시오.', '아니오'})
    if yes_or_no ~= 1 then
        return
    end

    local error = clan:leave(me)
    if error ~= nil then
        me:dialog(npc, error)
    else
        me:dialog(npc, '탈퇴 수속이 완료되었소.')
    end
end

return {
    on_click = function(me, npc)
        local clan = me:clan()
        if clan == nil then
            clan_registration(me, npc)
            return
        end

        local selected = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {'문파 정보', '문파 운영', '외교', '문파 탈퇴'})
        if selected == nil then
            return
        end

        clan = me:clan()
        if clan == nil then
            me:dialog(npc, '문파가 없구려.')
            return
        end

        if selected == 1 then
            clan_info(me, npc, clan)
        elseif selected == 2 then
            clan_operation(me, npc, clan)
        elseif selected == 3 then
            clan_diplomacy(me, npc, clan)
        elseif selected == 4 then
            clan_leave(me, npc, clan)
        end
    end
}
