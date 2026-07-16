-- npc: 설원조합사4
local EQUIP_BY_CLASS = {
    ["전사"] = "장갑",
    ["도적"] = "팔찌",
    ["주술사"] = "반지",
    ["도사"] = "선류",
}

local function get_equip_name(me)
    local class_name = class2name(me:class(), me:promotion())
    if not class_name then
        return "장갑"
    end
    return EQUIP_BY_CLASS[class_name] or "장갑"
end

local function run_snow_helmet_info(me, npc)
    local d = me:dialog(npc, "눈꽃투구에 대해 궁금하시다구요? 그렇다면 매우 잘 찾아오셨습니다. ", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "눈꽃투구는 매우 훌륭한 방어구임에 틀림 없지만, 그 특성상 함부로 사용하기가 어렵다는 단점이 있습니다.", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "갓 제작한 눈꽃투구는 그 한기가 너무 강력해, 착용자의 체력과 마력을 매우 많이 앗아간다는 치명적인 결함을 가지고 있지요.", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "하지만, 가공하기에 따라서는 그 결함 또한 최소화 할 수 있습니다.", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end

    local sel, btn = me:list(npc, "궁금한점은 해결이 되셨는지요?", { "눈꽃투구는 어떻게 가공할 수 있나요?", "알려주셔서 감사합니다." }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 2 then
        me:dialog(npc, "천만에요. 그럼 좋은하루 되세요~", { prev = false, next = false })
        return
    end
    if sel ~= 1 then
        return
    end

    d = me:dialog(npc, "아시다시피 눈꽃투구를 만들기 위해서는, 눈꽃얼음이 필요합니다.", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "이 눈꽃얼음은 압축하면 할 수록 단단해지고, 내부의 냉기가 밖으로 새어나오지 않는 특이한 성질을 가지고 있지요...", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "눈꽃투구의 결함은 총 열단계를 거쳐 개선될 수 있습니다.", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "첫번째 단계는 눈꽃얼음 10개, 두번째 단계에는 눈꽃얼음 20개, 그 다음 단계에는 30개가 필요하고, 그렇게 마지막 열단계 째에는 100개의 눈꽃얼음이 필요합니다.", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end

    sel, btn = me:list(npc, "만약 눈꽃투구와, 눈꽃얼음을 가져오셨다면 더 단단하게 압축하여 결함률을 줄여드리겠습니다.", { "예. 부탁드리겠습니다.", "아니오... 다음에..." }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 2 then
        me:dialog(npc, "언제든 마음이 바뀌면 다시 찾아오십시오.", { prev = false, next = false })
        return
    end
    if sel == 1 then
        me:dialog(npc, "눈꽃투구 가공 기능은 현재 준비 중입니다. 잠시 후 다시 찾아와 주세요.", { prev = false, next = false })
    end
end

function NPC_441(me, npc)
    local d = me:dialog(npc, "안녕하십니까? 당신은 눈꽃방어구에 대해 관심이 있으신 것 같군요. 자세한 설명을 들어보시겠습니까?", { prev = false, next = true })
    if d == DIALOG_RESULT.QUIT then
        return
    end

    local equip = get_equip_name(me)
    local sel, btn = me:list(npc, "눈꽃얼음은 그 한기가 매우 시려, 사용자의 체력과 마력을 약하게 만드는 단점을 가지고 있습니다.", {
        "눈꽃투구에 대하여...",
        "눈꽃" .. equip .. "에 대하여...",
        "활력의귀걸이에 대하여...",
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 1 then
        run_snow_helmet_info(me, npc)
        return
    end
    if sel == 2 or sel == 3 then
        me:dialog(npc, "준비중인 컨텐츠 입니다.", { prev = false, next = false })
    end
end