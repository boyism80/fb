-- npc: 직도
local MAP_ID_CLASS_HUB = 11004
local MAP_ID_WARRIOR = 11005
local MAP_ID_ROGUE = 11006
local MAP_ID_MAGE = 11007
local MAP_ID_TAOIST = 11008

local WARP_BY_NATION = {
    [NATION.GOGURYEO] = {
        { "국내성", 49, 37 },
        { "국내성", 22, 187 },
        { "국내성", 169, 62 },
        { "국내성", 185, 182 },
    },
    [NATION.BUYEO] = {
        { "부여성", 27, 26 },
        { "부여성", 22, 89 },
        { "부여성", 127, 36 },
        { "부여성", 97, 101 },
    },
}

local function do_warp(me, nation, class_index)
    local data = WARP_BY_NATION[nation] or WARP_BY_NATION[NATION.BUYEO]
    local t = data[class_index]
    if not t then
        return
    end
    local map = name2map(t[1])
    if map then
        me:map(map, t[2], t[3])
    end
end

function NPC_448(me, npc)
    local m = me:map()
    if m == nil then
        return
    end
    local map_id = m:model() and m:model():id() or 0

    if map_id == MAP_ID_CLASS_HUB then
        local btn = me:dialog(npc, "안녕하세요? 저는 직도라고 합니다. " .. me:name() .. "님께서는 직업에 대한 도움을 얻기 위해 오셨군요. 잘 오셨습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "이곳 직업의길에는, 각 직업에 대한 자세한 설명과 함께... 원하시는 직업을 얻을 수 있는 장소로 보내드리는 도움을 드리고 있답니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "전사에 대한 소개를 듣고싶으시면 [북쪽 ↑], 도적에 대한 소개를 듣고싶으시면 [서쪽 ←],", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "주술사에 대한 소개를 듣고 싶으시면 [동쪽 →] 도사에 대한 소개를 듣고 싶으시면 [남쪽 ↓]의 집에 들어가시면 됩니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "각 집의 안쪽에서는 제가 기다리고 있으니 너무 걱정하지 마시고 편하게 모든 집을 둘러보셔도 괜찮습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if map_id == MAP_ID_WARRIOR then
        local btn = me:dialog(npc, "전사에 대해 알아보러 오셨군요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "전사는 한번에 많은 양의 적들을 상대로 싸우고, 많은 경험치를 얻을 수 있는 직업입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "높은 체력을 바탕으로 선두에 서서, 방패 역할을 할 수 있는 든든한 직업이지요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "그러나... 귀환, 소환, 출두와 같은 순간이동 기술과 원거리 마법을 배울 수 없다는 단점이 있습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = me:list(npc, "전사의길로 보내드릴까요? 이동하신 뒤에 직업을 가지지 않는다면, 언제든 이곳으로 다시 돌아오실 수 있습니다.", { "전사의길로 가겠습니다.", "다른곳을 조금 더 둘러볼래요..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            do_warp(me, me:nation(), 1)
        else
            me:dialog(npc, "그럼 조금 더 다양한 직업들을 둘러보고 오세요.", { prev = false, next = true })
        end
        return
    end

    if map_id == MAP_ID_ROGUE then
        local btn = me:dialog(npc, "도적에 대해 알아보러 오셨군요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "도적은 하나의 적에게 집요하고 치명적인 공격을 가할 수 있는 직업입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "체력은 전사에 비해 낮지만, 다양한 기술을 바탕으로 전장을 흔들 수 있는 마치 창과 같은 직업입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "공성전과 같은 전장에서 빛을 발할 수 있는 직업이지만, 여럿에게 공격을 하기 힘들고 사냥속도가 전사보다 느리다는 단점이 있습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = me:list(npc, "도적의길로 보내드릴까요? 이동하신 뒤에 직업을 가지지 않는다면, 언제든 이곳으로 다시 돌아오실 수 있습니다.", { "도적의길로 가겠습니다.", "다른곳을 조금 더 둘러볼래요..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            do_warp(me, me:nation(), 2)
        else
            me:dialog(npc, "그럼 조금 더 다양한 직업들을 둘러보고 오세요.", { prev = false, next = true })
        end
        return
    end

    if map_id == MAP_ID_MAGE then
        local btn = me:dialog(npc, "주술사에 대해 알아보러 오셨군요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "주술사는 원거리 마법을 사용해, 다양한 공격 방법으로 적들에게 피해를 입힐 수 있는 직업입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "한명의 적에게 치명적인 일격을 가하는 [헬파이어]부터, 여럿의 적을 동시에 괴롭히는 [중독]과 같은 마법이 있습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "혼자서도 모든것을 할 수 있는 다재다능한 직업이지만, 그만큼 고독하고 외로운 직업입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "가장 많은 마법을 사용 가능하지만, 성장속도가 매우 느려 가장 난이도가 높은 직업이라고 볼 수 있습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = me:list(npc, "주술사의길로 보내드릴까요? 이동하신 뒤에 직업을 가지지 않는다면, 언제든 이곳으로 다시 돌아오실 수 있습니다.", { "주술사의길로 가겠습니다.", "다른곳을 조금 더 둘러볼래요..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            do_warp(me, me:nation(), 3)
        else
            me:dialog(npc, "그럼 조금 더 다양한 직업들을 둘러보고 오세요.", { prev = false, next = true })
        end
        return
    end

    if map_id == MAP_ID_TAOIST then
        local btn = me:dialog(npc, "도사에 대해 알아보러 오셨군요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "도사는 팀원을 보조해주는 마법을 사용하는 직업으로, 바람의나라에 없어서는 안될 매우 핵심적인 직업입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "적에게 [혼마술]을 사용해 방어력을 낮추어 아군이 적을 더 수월하게 공격할 수 있게 만들수도 있고,", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "아군에게 [기원]을 사용하여 체력을 보충해, 빠르게 전장에 재투입 될 수 있도록 응원할 수도 있습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "누구나 도사를 필요로 하기에 인기가 많고, 성장하기 또한 매우 수월합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "하지만 공격마법이 부족해, 혼자서는 무엇인가를 하기 어렵다는 단점을 가지고 있는것도 사실입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = me:list(npc, "도사의길로 보내드릴까요? 이동하신 뒤에 직업을 가지지 않는다면, 언제든 이곳으로 다시 돌아오실 수 있습니다.", { "도사의길로 가겠습니다.", "다른곳을 조금 더 둘러볼래요..." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            do_warp(me, me:nation(), 4)
        else
            me:dialog(npc, "그럼 조금 더 다양한 직업들을 둘러보고 오세요.", { prev = false, next = true })
        end
        return
    end

    me:dialog(npc, "준비중입니다.", { prev = false, next = false })
end