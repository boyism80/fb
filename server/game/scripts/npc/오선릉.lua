-- @brief   NPC Oh Seonreung (Seotdal house): gives 제기 during Chunghwa quest; optional exchange of 5 items for random reward (오선릉고물).
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity.
-- @note    When gv("sesi_rightnow") == 2 and QUEST_BAEK_MONGYEON step >= 1: gives 제기 once, or exchanges 연+깨진김장독+찌그러진냄비+널+실패 for one random item (once per character via quest progress).
function NPC_362(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 오선릉입니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if gv("sesi_rightnow") ~= 2 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    local quest = me:quest(QUEST_BAEK_MONGYEON)
    if quest == nil then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    if quest:step() < 1 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    local OSEONREUNG_REWARDS = { "티라노의알", "유리의장미", "유리의거울", "유리의팔찌", "유리의빗", "유리의안경" }
    local ACHIEVEMENT_OSEONREUNG = 56

    -- Already exchanged (오선릉고물 done): no more exchange, and if they have 제기 already just show 준비중
    if quest:progress() >= 1 then
        if me:has_items("제기", 1) then
            me:dialog(npc, "준비중입니다.", false, false)
            return
        end
        -- Give 제기 flow
        button = me:dialog(npc, "전 대청소가 정말 싫어요. 발명할때 사용할 재료들은 잡동사니가 아니란 말이에요! 저에겐 꼭 필요한건데도 몽연이형은 다 치우라고 하고..", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "음.. 그런데 몽연이형이 다른 집들에서 이것저것 받아달라고 했죠? 그거.. 저에게 주시면 제가 좋은 아이템 드릴 수 있는데...", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "몽연이형이 달라고 한 제기는 줘야겠죠....", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "칫. 몽연이형은 발명의 소중함을 모른단 말이야...", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        me:mkitem("제기", 1)
        return
    end

    -- Can exchange 5 items for random reward (연, 깨진김장독, 찌그러진냄비, 널, 실패)
    if me:has_items("연", 1) and me:has_items("깨진김장독", 1) and me:has_items("찌그러진냄비", 1) and me:has_items("널", 1) and me:has_items("실패", 1) then
        local reward_name = OSEONREUNG_REWARDS[math.random(1, 6)]
        button = me:dialog(npc, "연, 실패, 깨진김장독, 찌그러진냄비, 널을 가지고 계시군요! 그걸 저에게 주시면.. " .. name_with(reward_name, '을', '를') .. " 드릴 수 있는데...", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        local cost = { ['item'] = { ["연"] = 1, ["실패"] = 1, ["깨진김장독"] = 1, ["찌그러진냄비"] = 1, ["널"] = 1 } }
        local reward = { ['item'] = { [reward_name] = 1 } }
        local code = me:exchange(cost, reward)
        if code == EXCHANGE_RESULT.LACK_COST then
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 " .. name_with(reward_name, '을', '를') .. " 받을 수 없어요. 자리 좀 비우고 다시 오세요.", false, false)
            return
        end
        quest:progress(1)
        me:push_achievement(ACHIEVEMENT_OSEONREUNG, "오선릉에게 잡동사니를 전해주고 아이템을 받았다.", 7, 4)
        return
    end

    -- Give 제기 (no 제기 yet)
    if me:has_items("제기", 1) then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    button = me:dialog(npc, "전 대청소가 정말 싫어요. 발명할때 사용할 재료들은 잡동사니가 아니란 말이에요! 저에겐 꼭 필요한건데도 몽연이형은 다 치우라고 하고..", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "음.. 그런데 몽연이형이 다른 집들에서 이것저것 받아달라고 했죠? 그거.. 저에게 주시면 제가 좋은 아이템 드릴 수 있는데...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "몽연이형이 달라고 한 제기는 줘야겠죠....", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "칫. 몽연이형은 발명의 소중함을 모른단 말이야...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("제기", 1)
end
