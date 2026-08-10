-- npc: 백세인
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

local function craft_songpyeon(me, npc, filler_name, result_name, missing_filler_msg, success_msg)
    if not me:has_items(filler_name, 1) then
        me:dialog(npc, missing_filler_msg, { prev = false, next = false })
        return
    end
    if not me:has_items("깨", 1) then
        me:dialog(npc, "깨가 없으신데요?", { prev = false, next = false })
        return
    end
    if not me:has_items("쌀가루", 1) then
        me:dialog(npc, "쌀가루가 없으신데요?", { prev = false, next = false })
        return
    end

    local code = me:exchange(
        { ['item'] = { [filler_name] = 1, ["깨"] = 1, ["쌀가루"] = 1 } },
        { ['item'] = { [result_name] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "재료가 부족합니다.", { prev = false, next = false })
        return
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 송편을 드릴 수 없습니다.", { prev = false, next = false })
        return
    end
    me:dialog(npc, success_msg, { prev = false, next = false })
end

local function run_chuseok(me, npc)
    local btn = me:dialog(npc, "안녕하세요! 신나는 추석이에요~!!", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "제가 도와드릴 일이 있나요?", {
        "추석엔 뭘 하나요?",
        "송편을 만들어주세요!",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        btn = me:dialog(npc, "추석(秋夕)은 가을의 시작을 알리며, 햇과일과 햇곡식이 풍성하여 넉넉하고 흥겨운 날이에요. '한가위'라고도 하는데요, '한'은 크다는 뜻이고, '가위'는 가운데라는 뜻이에요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "추석무렵은 일년중 가장 좋은 시절이고 만물이 풍성한 때여서, 예로부터 '더도 말고 덜도 말고 늘 가윗날만 같아라.'라는 말도 있죠", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "추석에는 송편을 만들어 먹어야죠! 유두집의 제 동생 리향에게 송편재료가 있을꺼에요. 받아오시면 제가 송편을 만들어드릴께요~", { prev = false, next = false })
        return
    end

    if sel == 2 then
        btn = me:dialog(npc, "송편 재료를 받아오셨어요? 송편에는 여러가지 재료가 들어갈 수 있는데.. 어떤 재료를 받아오셨어요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local mat_sel, mat_btn = me:list(npc, "구해오신 재료가 뭐에요?", {
            "밤을 구해왔어요.",
            "호두를 구해왔어요.",
            "호박을 구해왔어요.",
        }, { prev = false })
        if mat_btn == DIALOG_RESULT.QUIT then
            return
        end
        if mat_sel == nil then
            return
        end

        if mat_sel == 1 then
            craft_songpyeon(me, npc, "밤", "밤송편", "밤이 없으신데요?", "밤을 넣어 만든 송편을 만들어 드렸습니다~ 맛있게 드세요!")
        elseif mat_sel == 2 then
            craft_songpyeon(me, npc, "호두", "호두송편", "호두가 없으신데요?", "호두를 넣어 만든 송편을 만들어 드렸습니다~ 맛있게 드세요!")
        elseif mat_sel == 3 then
            craft_songpyeon(me, npc, "식용호박", "호박송편", "식용호박이 없으신데요?", "식용호박을 넣어 만든 송편을 만들어 드렸습니다~ 맛있게 드세요!")
        end
    end
end

local function run_junghwa(me, npc)
    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end
    if me:has_items("찌그러진냄비", 1) then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    local button = me:dialog(npc, "몽연이가 청소를 한다구요?", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "작년 추석에 송편 만들다가 냄비를 떨어뜨려서 냄비가 찌그러졌는데.. 잘됐네요! 나중에 선릉이한테 냄비 새로 만들어 달래야지~", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "감사합니다!!", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("찌그러진냄비", 1)
end

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요? 저는 백세인입니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('추석') then
            run_chuseok(me, npc)
        elseif festival.is('중화절') then
            run_junghwa(me, npc)
        else
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        end
    end
}
