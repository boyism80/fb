local M = {}

local YES = '예'
local NO = '아니오'

local function queues()
    return {
        { type = MATCH_TYPE.MATCH_1, label = '매치 1' },
        { type = MATCH_TYPE.MATCH_2, label = '매치 2' },
    }
end

local function is_group_master(me)
    local group = me:group()
    if group == nil then
        return true
    end
    return group:master() == me:name()
end

function M.handle(me, npc)
    local mm = me:matchmaker()

    if not is_group_master(me) then
        me:dialog(npc, '그룹장만 매치메이킹을 이용할 수 있습니다.', { prev = false, next = true })
        return true
    end

    if mm:enrolled() then
        local selected, button = me:pursuit(npc, '현재 등록된 매칭이 있습니다. 취소하시겠습니까?', { YES, NO })
        if button == DIALOG_RESULT.QUIT then
            return true
        end
        if selected == YES then
            local err = mm:unregister()
            if err ~= nil then
                me:dialog(npc, err, { prev = false, next = true })
            end
        end
        return true
    end

    local labels = {}
    for _, queue in ipairs(queues()) do
        labels[#labels + 1] = queue.label
    end

::MATCHMAKING_TYPE::
    local selected, button = me:pursuit(npc, '매치 유형을 선택해 주세요.', labels)
    if button == DIALOG_RESULT.QUIT then
        return true
    end

    local queue = nil
    for _, q in ipairs(queues()) do
        if q.label == selected then
            queue = q
            break
        end
    end
    if queue == nil then
        return true
    end

    local confirm_selected, confirm_button = me:pursuit(npc, string.format('"%s" 매치를 등록하시겠습니까?', queue.label), { YES, NO })
    if confirm_button == DIALOG_RESULT.QUIT then
        return true
    end
    if confirm_selected ~= YES then
        return true
    end

    local err = mm:register(queue.type)
    if err ~= nil then
        me:dialog(npc, err, { prev = false, next = true })
        return true
    end

    return false
end

return M
