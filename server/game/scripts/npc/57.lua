-- npc: 얼음돌이
local enum = require('lib.enum')

local ICE_NAME = '얼음'
local CLEAN_ICE_NAME = '깨끗한얼음'
local SMALL_ICE_NAME = '작은얼음'
local MAX_SPLIT = 10

local function run_ice_wash(me, npc)
    ::NPC_57_0001::
    local btn = me:dialog(npc, '땅에 떨어져 지저분해진 얼음을 깨끗이 세척해드립니다.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end
    if btn == DIALOG_RESULT.PREV then
        return DIALOG_RESULT.PREV
    end
    if not me:has_items(ICE_NAME, 1) then
        btn = me:dialog(npc, '얼음을 구해오시면 씻어드리지요.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return nil
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_57_0001
        end
        return nil
    end
    ::NPC_57_0002::
    btn = me:dialog(npc, '여기 흐르는 물에 씻어드리겠습니다.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_57_0001
    end
    local r = math.random(1, 5)
    local reward = (r == 2 or r == 4) and { ['item'] = { [CLEAN_ICE_NAME] = 1 } } or nil
    local code = me:exchange(
        { ['item'] = { [ICE_NAME] = 1 } },
        reward
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        btn = me:dialog(npc, '얼음을 구해오시면 씻어드리지요.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_57_0001
        end
        return nil
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        btn = me:dialog(npc, '소지품이 가득 차서 깨끗한얼음을 받을 수 없어요.', { prev = false, next = true })
        return nil
    end
    if reward ~= nil then
        btn = me:dialog(npc, '얼음을 씻어드렸습니다.', { prev = false, next = true })
    elseif r == 1 then
        btn = me:dialog(npc, '저런, 얼음이 녹아 없어졌네요.', { prev = false, next = true })
    elseif r == 3 then
        btn = me:dialog(npc, '물이 따뜻해서 얼음이 녹아버렸습니다.', { prev = false, next = true })
    else
        btn = me:dialog(npc, '얼음이 물에 빠져서 녹았습니다.', { prev = false, next = true })
    end
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end
    return nil
end

local function run_ice_split(me, npc)
    if not me:has_items(ICE_NAME, 1) then
        local btn = me:dialog(npc, '얼음을 구해오세요.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return nil
        end
        if btn == DIALOG_RESULT.PREV then
            return DIALOG_RESULT.PREV
        end
        return nil
    end
    local msg = '1개의 얼음을 쪼개면 작은얼음 10개가 됩니다.\n작은얼음은 100개까지 소지 가능하므로, 최대 10개의 얼음을 쪼개드릴 수 있겠네요.\n몇 개의 얼음을 쪼개시겠어요?'
    local raw = me:input(npc, msg)
    if raw == nil then
        return nil
    end
    local num = tonumber(raw)
    if num == nil or num < 1 or num > MAX_SPLIT then
        local btn = me:dialog(npc, '올바른 개수를 입력해주세요.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            return DIALOG_RESULT.PREV
        end
        return nil
    end
    local code = me:exchange(
        { ['item'] = { [ICE_NAME] = num } },
        { ['item'] = { [SMALL_ICE_NAME] = num * 10 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        local btn = me:dialog(npc, '얼음이 부족합니다.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            return DIALOG_RESULT.PREV
        end
        return nil
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        local btn = me:dialog(npc, '소지품이 가득 차서 작은얼음을 받을 수 없어요.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            return DIALOG_RESULT.PREV
        end
        return nil
    end
    return nil
end

return {
    on_click = function(me, npc)
        ::NPC_57_START::
        local sel, list_btn = me:list(npc, '안녕하세요. 어떻게 오셨나요?', { '얼음세척', '얼음을 쪼개주세요' })
        if list_btn == DIALOG_RESULT.PREV then
            return
        end
        if sel == nil then
            return
        end
        if sel == 1 then
            local r = run_ice_wash(me, npc)
            if r == DIALOG_RESULT.PREV then
                goto NPC_57_START
            end
        else
            local r = run_ice_split(me, npc)
            if r == DIALOG_RESULT.PREV then
                goto NPC_57_START
            end
        end
    end
}
