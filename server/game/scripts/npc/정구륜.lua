function NPC_6(me, npc)
    local job_name = class2name(me:class(), me:promotion())
    if not job_name then
        job_name = '주술사'
    end
    local sel = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {
        name_with(job_name, '으로서', '로서') .. ' 승급을 원합니다',
        '기술을 배울래요',
        '3차승급 옷을 원합니다',
    })
    if sel == nil then
        return
    end
    if sel == 0 then
        if me:promotion() >= 3 then
            me:dialog(npc, '당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.', false, true)
        else
            NPC_PROMOTION(me, npc, CLASS.MAGE)
        end
        return
    end
    if sel == 1 then
        NPC_PROMOTION_SKILLS(me, npc, CLASS.MAGE)
        return
    end
    if sel == 2 then
        NPC_PROMOTION_CLOTHES(me, npc, CLASS.MAGE)
    end
end
