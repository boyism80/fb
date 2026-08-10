-- npc: 촌장
local enum = require('lib.enum')

local ACHIEVEMENT_SESIHELMET = 564

local FIXED_MATERIALS = {
    '중화절부적',
    '버들피리',
    '연등',
    '비녀',
    '유두면',
    '올게심니',
    '약신의물병',
    '고사반',
    '고약',
    '복조리',
}

local function pick_seolbim(me)
    if me:has_items('남자설빔', 1) then
        return '남자설빔'
    end
    if me:has_items('여자설빔', 1) then
        return '여자설빔'
    end
    return nil
end

local function pick_chilseok(me)
    if me:has_items('견우의피리', 1) then
        return '견우의피리'
    end
    if me:has_items('직녀의바늘', 1) then
        return '직녀의바늘'
    end
    return nil
end

local function missing_materials(me)
    local missing = {}
    if pick_seolbim(me) == nil then
        missing[#missing + 1] = '설빔'
    end
    for i = 1, #FIXED_MATERIALS do
        local name = FIXED_MATERIALS[i]
        if not me:has_items(name, 1) then
            missing[#missing + 1] = name
        end
    end
    if pick_chilseok(me) == nil then
        missing[#missing + 1] = '견우의피리/직녀의바늘'
    end
    return missing
end

local function run_helmet_craft(me, npc)
    local missing = missing_materials(me)
    if #missing > 0 then
        local btn = me:dialog(npc, "세시풍습 아이템 12가지를 모두 모아 오시면 세시투구를 만들어 드리겠습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "아직 부족한 재료가 있습니다.\n\n" .. table.concat(missing, ', '), { prev = false, next = false })
        return
    end

    local btn = me:dialog(npc, "오호, 세시풍습 아이템을 모두 모으셨군요. 세시투구를 만들어 드릴까요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local seolbim = pick_seolbim(me)
    local chilseok = pick_chilseok(me)
    local cost = {
        [seolbim] = 1,
        [chilseok] = 1,
    }
    for i = 1, #FIXED_MATERIALS do
        cost[FIXED_MATERIALS[i]] = 1
    end

    local code = me:exchange(
        { ['item'] = cost },
        { ['item'] = { ['세시투구'] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "세시풍습 아이템 12가지를 모두 모아 오셔야 합니다.", { prev = false, next = false })
        return
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 세시투구를 드릴 수 없습니다.", { prev = false, next = false })
        return
    end

    me:push_achievement(ACHIEVEMENT_SESIHELMET, "세시투구를 제작해보자", 7, 1)
    me:dialog(npc, "자, 여기 세시투구입니다. 오래오래 간직하시게.", { prev = false, next = true })
end

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요.. 여기는 세시마을 입니다. 저는 촌장 백도원이라고 합니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local sel, list_btn = me:list(npc, "무슨 일로 오셨습니까?", {
            "세시투구를 만들고 싶어요.",
            "그냥 인사차 왔습니다.",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end

        if sel == 1 then
            run_helmet_craft(me, npc)
        else
            me:dialog(npc, "세시마을에서 편안히 쉬다 가십시오.", { prev = false, next = false })
        end
    end
}
