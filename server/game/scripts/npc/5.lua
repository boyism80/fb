-- npc: 은구륜
local npc = require('lib.npc')

function NPC_5(me, npc_obj)
    local class_name = class2name(me:class(), me:promotion())
    if not class_name then
        class_name = '도사'
    end
    local sel = me:list(npc_obj, '안녕하세요. 어떻게 오셨나요?', {
        name_with(class_name, '으로서', '로서') .. ' 승급을 원합니다',
        '기술을 배울래요',
        '3차승급 옷을 원합니다',
    })
    if sel == nil then
        return
    end
    if sel == 1 then
        if me:promotion() >= 3 then
            me:dialog(npc_obj, '당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.', { prev = false, next = true })
        else
            npc.promotion(me, npc_obj, CLASS.POET)
        end
        return
    end
    if sel == 2 then
        npc.promotion_skills(me, npc_obj, CLASS.POET)
        return
    end
    if sel == 3 then
        npc.promotion_clothes(me, npc_obj, CLASS.POET)
    end
end