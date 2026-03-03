
local rewards = {
    [1] = { threshold = 10, items = { { name = '국내성비서', count = 5 }, { name = '장안성비서', count = 5 } } },
    [2] = { threshold = 50, items = { { name = '토깽이변신시약', count = 1 } } },
    [3] = { threshold = 100, items = { { name = '청의태자변신시약', count = 1 } } },
    [4] = { threshold = 1000, items = { { name = '회호박결정', count = 1 } } },
    [5] = { threshold = 10000, items = { { name = '천풍선', count = 1 } } },
}

function seocheon_give_tier_reward(me, step)
    local tier = rewards[step]
    if tier == nil or tier.items == nil then
        return nil
    end
    local items_table = {}
    for _, it in ipairs(tier.items) do
        items_table[it.name] = it.count
    end
    return me:mkitem(items_table) ~= nil
end

function NPC_215(me, npc)
    local quest = me:quest(QUEST_GREATWALL)

    if quest == nil then
        ::NPC_215_COS001::
        local btn = me:dialog(npc, '마침 잘 만났네. 자네 만리장성을 돌아보았나? 요즘 만리장성의 곳곳이 무너져내려 걱정이 보통이 아니라네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_215_COS002::
        btn = me:dialog(npc, '만리장성을 수리하는데는 고구려의 벽돌만큼 좋은게 없지.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_215_COS001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, lb = me:list(npc, '자네가 만리장성의 무너져내린곳을 고쳐줄 수 있겠나?', { '만리장성이!! 제가 꼭 고치겠습니다!!', '죄송하지만 저에겐 그럴만한 시간이 없습니다.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        quest = me:start_quest(QUEST_GREATWALL)
        if quest == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest:step(1)
        me:push_achievement(21, '만리장성 수리를 부탁받다.', 7, 1)
        me:dialog(npc, '수리에는 꼭 고구려의 벽돌을 써야만 하네. 벽돌은 국내성의 대장간에서 판다고 하더군. 만리장성을 돌아다니다 보면 수리해야 할곳을 찾을수 있을걸세. 10군데 이상 고친다면 적절한 보상을 해주겠네.', true, true)
        return
    end

    if quest:step() == 0 then
        ::NPC_215_COS001B::
        local btn = me:dialog(npc, '마침 잘 만났네. 자네 만리장성을 돌아보았나? 요즘 만리장성의 곳곳이 무너져내려 걱정이 보통이 아니라네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_215_COS002B::
        btn = me:dialog(npc, '만리장성을 수리하는데는 고구려의 벽돌만큼 좋은게 없지.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_215_COS001B
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, lb = me:list(npc, '자네가 만리장성의 무너져내린곳을 고쳐줄 수 있겠나?', { '만리장성이!! 제가 꼭 고치겠습니다!!', '죄송하지만 저에겐 그럴만한 시간이 없습니다.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        quest:step(1)
        me:push_achievement(21, '만리장성 수리를 부탁받다.', 7, 1)
        me:dialog(npc, '수리에는 꼭 고구려의 벽돌을 써야만 하네. 벽돌은 국내성의 대장간에서 판다고 하더군. 만리장성을 돌아다니다 보면 수리해야 할곳을 찾을수 있을걸세. 10군데 이상 고친다면 적절한 보상을 해주겠네.', true, true)
        return
    end

    if quest:completed() then
        me:dialog(npc, '저번엔 정말 고마웠네. 당신과 같이 만리장성의 수리에 힘써주는 사람이 많아 만리장성이 점차 옛 모습을 되찾고 있네.', true, true)
        return
    end

    local step = quest:step()
    local progress = quest:progress()
    local tier = rewards[step]
    if tier == nil then
        me:dialog(npc, '저번엔 정말 고마웠네. 당신과 같이 만리장성의 수리에 힘써주는 사람이 많아 만리장성이 점차 옛 모습을 되찾고 있네.', true, true)
        return
    end

    if progress < tier.threshold then
        me:dialog(npc, '조금 더 수리해야 할 것 같은데?', false, true)
        return
    end
    if seocheon_give_tier_reward(me, step) == nil then
        me:dialog(npc, '소지품이 가득 차서 보상을 받을 수 없습니다.', false, true)
        return
    end

    local next_step = step + 1
    quest:step(next_step)
    if rewards[step + 1] == nil then
        quest:complete()
        me:push_achievement(21, '만리장성 수리에 성공하다.', 7, 1)
    end
    me:dialog(npc, '오~ 수고했네!! 자 여기 받게나. 시간이 있다면 더 수고해 주길 바라겠네. 수고하는 만큼 댓가는 지불할테니...', true, true)
end