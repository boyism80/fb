-- npc: 천성랑
local npc = require('lib.npc')
local quest = require('lib.quest')

local QUEST_STATS = {
    [CLASS.WARRIOR] = { min_hp = 600000, min_mp = 0 },
    [CLASS.ROGUE]   = { min_hp = 600000, min_mp = 0 },
    [CLASS.MAGE]    = { min_hp = 300000, min_mp = 500000 },
    [CLASS.POET]    = { min_hp = 250000, min_mp = 250000 },
}

local function meets_4th_promotion_quest_requirements(me)
    if me:promotion() < 3 then
        return false, '3차 승급을 한 뒤에 다시 오세요.'
    end
    if me:level() < 99 then
        return false, '좀더 수련을 쌓고오세요.'
    end
    local stats = QUEST_STATS[me:class()]
    if not stats then
        return false, '아직 준비가 안 되었습니다.'
    end
    if me:base_hp() < stats.min_hp or me:base_mp() < stats.min_mp then
        return false, '좀더 수련을 쌓고오세요.'
    end
    return true
end

return {
    on_click = function(me, npc_obj)
        local selected, button = me:pursuit(npc_obj, '안녕하세요. 어떻게 오셨나요?', {
            '4차 승급을 원합니다',
        })
        if button == DIALOG_RESULT.QUIT or selected ~= 1 then
            return
        end
        if me:promotion() >= 4 then
            local done = me:quest(quest.QUEST_MUTA)
            if done and not done:completed() and done:step() >= 7 then
                done:complete()
            end
            me:dialog(npc_obj, '당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.', { prev = false, next = false })
            return
        end

        local q = me:quest(quest.QUEST_MUTA)
        if q and (q:completed() or q:step() >= 7) then
            if me:promotion() < 3 then
                me:dialog(npc_obj, '3차 승급을 한 뒤에 다시 오세요.', { prev = false, next = false })
                return
            end
            local waived = q:completed() and not me:has_items('반고의심장', 1)
            if not waived and not me:has_items('반고의심장', 1) then
                me:dialog(npc_obj, '반고의심장이 없습니다.', { prev = false, next = false })
                return
            end
            npc.promotion_4th(me, npc_obj, not waived, q)
            return
        end

        local ok, err_msg = meets_4th_promotion_quest_requirements(me)
        if not ok then
            me:dialog(npc_obj, err_msg, { prev = false, next = false })
            return
        end
        local started = me:start_quest(quest.QUEST_MUTA)
        if started == nil then
            me:dialog(npc_obj, '퀘스트를 시작할 수 없습니다.', { prev = false, next = false })
            return
        end
        me:dialog(npc_obj,
            '기본적인 능력은 갖추셨습니다만, 그만한 지위에 오르시려거든 세상을 위한 업적을 세우셔야 합니다.\n\n소문에, 천인(天人)으로 추앙받고 계신 무타님께서 세상에 닥쳐올 재앙에 대항할 인재를 구하신다고 하던데 한 번 찾아가보시는 것이 어떨까요?',
            { prev = false, next = false })
    end
}
