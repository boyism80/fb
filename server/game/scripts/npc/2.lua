-- npc: 랑구륜
local quest = require('lib.quest')
local enum = require('lib.enum')

local function ranggyuryun_palgu(me, npc)
    local EIGHT_TRIGRAMS = { '건괘', '곤괘', '진괘', '감괘', '리괘', '태괘', '선괘', '간괘' }
    local btn, sel

    ::NPC_2_0001::
    btn = me:dialog(npc, '어서 오십시요. 저는 옥황상제의 막내딸 랑구륜이랍니다.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_2_0010::
    btn = me:dialog(npc, '저는 8개의 괘를 팔괘로 바꾸어드립니다.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_2_0001
    end
    sel = me:list(npc, '8개의 괘를 팔괘로 바꾸시겠어요?', { '네', '아니요, 팔괘가 다 없어요...' })
    if sel == nil then
        return
    end
    if sel == 2 then
        me:dialog(npc, '다음엔 팔괘를 다 모아오세요...', { prev = false, next = false })
        return
    end
    if sel ~= 1 then
        return
    end
    local materials = {}
    for i = 1, 8 do
        materials[EIGHT_TRIGRAMS[i]] = 1
    end
    if not me:has_items(materials) then
        me:dialog(npc, '아직 8개의 괘를 다 모으지 못하셨군요...', { prev = false, next = false })
        return
    end
    ::NPC_2_0020::
    btn = me:dialog(npc, '8개의 괘들을 다 가져오셨군요. 팔괘를 만들어 드리겠습니다, 잠시만 기달려주세요.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_2_0021::
    btn = me:dialog(npc, '자 팔괘를 만들어 드렸습니다. 그럼 안녕히가십시요.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local code = me:exchange(
        { ['item'] = materials },
        { ['item'] = { ['팔괘'] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, '아직 팔괘 재료를 다 모으지 못하셨군요.', { prev = false, next = true })
        return
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 팔괘를 받을 수 없습니다.', { prev = false, next = true })
        return
    end
end

local function ranggyuryun_pure_water(me, npc)
    local ACHIEVEMENT_CLEAR = 23
    local q = me:quest(quest.QUEST_CLEAR_SHIELD)
    local btn, sel

    if q == nil or (q:step() ~= 1 and q:step() ~= 2) then
        me:dialog(npc, '아, 홍옥의 그 광채...빛깔...맛...언제 생각해도 황홀하네.', { prev = false, next = false })
        return
    end

    if q:step() == 1 then
        sel = me:list(npc, ' ', { '물을 정화시키는 방법을 아시나요?' })
        if sel == nil or sel ~= 1 then return end
        sel = me:list(npc, '물론 알고 있지요. 하지만 그 방법을 배우기 위해서는 대가가 필요하지요.', { '무슨 대가인가요?' })
        if sel == nil or sel ~= 1 then return end
        sel = me:list(npc, '신선한 사과를 먹어본지 참 오래 되었는데..가서 홍옥 3개만 가지고 오세요.', { '예. 알겠습니다.', '홍옥!! 차라리 내가 먹고 말지..' })
        if sel == nil or sel ~= 1 then return end
        q:step(2)
        me:push_achievement(ACHIEVEMENT_CLEAR, '랑구륜의 부탁을 들어주자.', 7, 1)
        me:dialog(npc, '아참 전 국광보다는 홍옥을 좋아하니 꼭 홍옥으로 3개를 가져오세요.', { prev = false, next = false })
        return
    end

    if q:step() == 2 then
        local materials = {['홍옥'] = 3}
        if not me:has_items(materials) then
            me:dialog(npc, '아직 홍옥 3개를 구하시지 못하신거군요.', { prev = false, next = false })
            return
        end
        btn = me:dialog(npc, '어머 홍옥을 가져오셨군요. 이건 제가 잘 먹을께요.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = materials },
            { ['item'] = { ['정화비서'] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, '아직 홍옥 3개를 구하시지 못하신거군요.', { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 정화비서를 받을 수 없습니다.', { prev = false, next = true })
            return
        end
        q:step(3)
        me:push_achievement(ACHIEVEMENT_CLEAR, '랑구륜의 부탁을 들어주었다.', 7, 1)
        ::NPC_2_0030::
        btn = me:dialog(npc, '우물우물... 아... 역시 언제 먹어도 홍옥의 맛이 최고야.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '정화비서를 가져다 주면 될거에요.', { prev = false, next = false })
        return
    end
end

local function ranggyuryun_golden_amber(me, npc)
    local btn = me:dialog(npc, '호박의 정수.. 황금호박별을 가지고 오셨습니까. 이것으로 무기를 만드시렵니까..', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then return end
    if not me:has_items('황금호박별', 1) then
        me:dialog(npc, '황금호박별을 가지고 있지 않으시군요. 아직은 때가 아닌가보군요..', { prev = false, next = false })
    end
end

return {
    on_click = function(me, npc)
        local sel = me:list(npc, '안녕하세요. 어떻게 오셨나요?', { '팔괘', '순수한물', '황금호박무기만들기' })
        if sel == nil then return end
        if sel == 1 then
            ranggyuryun_palgu(me, npc)
            return
        end
        if sel == 2 then
            ranggyuryun_pure_water(me, npc)
            return
        end
        if sel == 3 then
            ranggyuryun_golden_amber(me, npc)
        end
    end
}
