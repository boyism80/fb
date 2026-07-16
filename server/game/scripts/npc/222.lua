-- npc: 주례사
local MARRIAGE_MIN_LEVEL = 21

local function marriage_npc(me, npc)
    local m = me:marriage()
    local selected = me:list(npc, '안녕하세요. 어떻게 오셨나요?', { '결혼을 하려고 왔습니다', '이혼을 하려고 왔습니다' })
    if selected == nil then
        return
    end

    if selected == 1 then
        if m.married then
            me:dialog(npc, '이미 결혼을 하셨습니다.')
            return
        end
        if not m.can_remarry then
            me:dialog(npc, '재혼 대기 시간이 지나지 않았습니다. 잠시 후에 다시 시도해 주세요.')
            return
        end
        if me:level() < MARRIAGE_MIN_LEVEL then
            me:dialog(npc, string.format('레벨 %d 이상만 결혼할 수 있습니다.', MARRIAGE_MIN_LEVEL))
            return
        end

        local name = me:input(npc, '결혼하실 이름을 입력해 주십시오.')
        if name == nil or name == '' then
            return
        end

        local map = me:map()
        if map == nil then
            return
        end
        local nears = map:nears({ me:position() }, OBJECT_TYPE.CHARACTER)
        local found = nil
        for _, ch in pairs(nears) do
            if ch:name() == name then
                found = ch
                break
            end
        end

        if found == nil then
            me:dialog(npc, name .. '님은 현재 접속 중이 아닙니다.')
            return
        end

        if found:uid() == me:uid() then
            me:dialog(npc, '자신과는 결혼할 수 없습니다.')
            return
        end

        if found:gender() == me:gender() then
            me:dialog(npc, '같은 성별끼리는 결혼할 수 없습니다.')
            return
        end

        local target_m = found:marriage()
        if target_m.married then
            me:dialog(npc, '이미 결혼한 상대와는 결혼을 할 수 없습니다.')
            return
        end
        if not target_m.can_remarry then
            me:dialog(npc, '상대방은 아직 재혼할 수 없습니다.')
            return
        end
        if found:level() < MARRIAGE_MIN_LEVEL then
            me:dialog(npc, string.format('결혼할 상대의 레벨이 %d 이상이어야 합니다.', MARRIAGE_MIN_LEVEL))
            return
        end

        local yes_or_no = found:menu(npc, string.format('정말 %s님과 결혼하시겠습니까?', me:name()), { '예', '아뇨' })
        if yes_or_no == 1 then
            local err = me:marry(found)
            if err ~= nil then
                me:dialog(npc, err)
                found:dialog(npc, err)
            else
                me:dialog(npc, string.format('%s님과 결혼했습니다.', found:name()))
                found:dialog(npc, string.format('%s님과 결혼했습니다.', me:name()))
            end
        else
            me:dialog(npc, string.format('%s님이 거절하셨습니다.', found:name()))
        end
    elseif selected == 2 then
        if not m.married then
            me:dialog(npc, '결혼을 한 사람만이 이혼을 할 수 있습니다.')
            return
        end

        local spouse = id2ch(m.spouse_id)
        if spouse == nil then
            me:dialog(npc, m.spouse_name .. '님은 현재 접속 중이 아닙니다.')
            return
        end

        local yes_or_no = spouse:menu(npc, string.format('%s님이 이혼을 요청했습니다. 수락하시겠습니까?', me:name()), { '예', '아뇨' })
        if yes_or_no == 1 then
            local err = me:divorce()
            if err ~= nil then
                me:dialog(npc, err)
                spouse:dialog(npc, err)
            else
                me:dialog(npc, '이혼이 완료되었습니다. 7일 후 재혼이 가능합니다.')
                spouse:dialog(npc, '이혼이 완료되었습니다. 7일 후 재혼이 가능합니다.')
            end
        else
            me:dialog(npc, string.format('%s님이 거절하셨습니다.', spouse:name()))
        end
    end
end

function NPC_222(me, npc)
    marriage_npc(me, npc)
end