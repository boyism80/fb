-- npc: 백리향
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

local function run_songpyeon_request(me, npc, q)
    local btn = me:dialog(npc, "송편재료가 필요하시다구요? 아... 세인이 언니에게 얘기듣고 오셨나보군요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "음.. 제가 지금 벌초를 해야하는데.. 대신 해주시면 송편재료를 드릴께요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "저희마을 서쪽에 가면 묘지가 있는데.. 무덤주위에 잡초들이 자라 있거든요.. 잡초들을 좀 없애주세요..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "그럼 부탁드릴께요~~~", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if q == nil then
        q = me:start_quest(quest.QUEST_BAEKRIHYANG)
        if q == nil then
            return
        end
    end
    if q and not q:completed() then
        q:step(1)
        q:progress(0)
        festival.stamp_lunar_year(q)
    end
end

local function run_weeding_handin(me, npc, q)
    if q == nil or q:step() < 1 then
        me:dialog(npc, "벌초를 하지 않으셨네요? 절 속이시면 안돼죠..", { prev = false, next = false })
        return
    end
    if q:progress() < 1 then
        me:dialog(npc, "벌초를 하지 않으셨네요? 절 속이시면 안돼죠..", { prev = false, next = false })
        return
    end
    if q:completed() then
        me:dialog(npc, "이미 송편재료를 받으셨죠. 세인이 언니에게 가서 맛있는 송편 만들어달라고 하세요. ^^", { prev = false, next = false })
        return
    end

    local btn = me:dialog(npc, "어머.. 벌초를 하고 오셨나요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "아.. 감사합니다! 그럼 약속 드린대로 재료를 드릴께요..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "그럼 세인이언니에게 가서 맛있는 송편 만들어달라고 하세요. ^^", { prev = false, next = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local r = math.random(1, 10)
    local items
    if r <= 4 then
        items = { ["호두"] = 1, ["깨"] = 1, ["쌀가루"] = 1 }
    elseif r <= 8 then
        items = { ["밤"] = 1, ["깨"] = 1, ["쌀가루"] = 1 }
    else
        items = { ["식용호박"] = 1, ["깨"] = 1, ["쌀가루"] = 1 }
    end
    if me:mkitem(items) == nil then
        me:dialog(npc, "소지품이 가득 차서 재료를 줄 수 없습니다.", { prev = false, next = false })
        return
    end
    q:progress(0)
    q:complete()
    festival.mark_completed_year(q)
end

local function run_chuseok(me, npc)
    local q = quest.get_annual(me, quest.QUEST_BAEKRIHYANG)
    local sel, list_btn = me:list(npc, "무슨일로 절 찾아오셨나요?", {
        "송편재료가 필요해요.",
        "벌초를 하고 왔어요."
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        run_songpyeon_request(me, npc, q)
    else
        run_weeding_handin(me, npc, q)
    end
end

local function explain_yudu(me, npc)
    local btn = me:dialog(npc, "유두절(流頭節)은 음력 6월 15일이에요. 일가 친지들과 함께 시내나 폭포에 가서 몸을 씻고 서늘하게 하루를 보내는 날이죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "음.. 길림성에 가시면 폭포가 있는데, 거기서 목욕을 한번 해보세요. 그리고 저희 마을 옆에 있는 밭에서 피, 조, 벼, 콩을 구해 오시면 제가 유두면을 만들어 드릴게요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "매달마다 구할 수 있는 아이템을 모두 모으면 좋은 일이 생길 거에요. 유두절에는 유두면이 그 아이템입니다~", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc, "그리고, 저희 마을 칠성당에서는 올한해 농사가 잘되라는 고사도 지낸답니다. 한번 가보세요~", { prev = false, next = false })
end

local function make_yudumyeon(me, npc)
    if not me:has_items("벼", 1) or not me:has_items("콩", 1) or not me:has_items("피", 1) or not me:has_items("조", 1) then
        me:dialog(npc, "저희 마을의 논과 밭에서 피, 조, 벼, 콩을 구해오셔요. 길림성의 폭포에서 목욕하시는 것도 잊지 마시구요.", { prev = false, next = false })
        return
    end

    local btn = me:dialog(npc, "다 구해오셨군요? 그럼 제가 유두면을 만들어드릴게요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ["벼"] = 1, ["콩"] = 1, ["피"] = 1, ["조"] = 1 } },
        { ['item'] = { ["유두면"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "저희 마을의 논과 밭에서 피, 조, 벼, 콩을 구해오셔요. 길림성의 폭포에서 목욕하시는 것도 잊지 마시구요.", { prev = false, next = false })
        return
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 유두면을 드릴 수 없습니다.", { prev = false, next = false })
        return
    end

    btn = me:dialog(npc, "매달마다 구할 수 있는 아이템을 모두 모으면 좋은 일이 생길 거에요. 유두절에는 유두면이 그 아이템입니다~", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc, "그리고, 저희 마을 칠성당에서는 올한해 농사가 잘되라는 고사도 지낸답니다. 한번 가보세요~", { prev = false, next = false })
end

local function run_yudu(me, npc)
    local sel, list_btn = me:list(npc, "무엇을 도와드릴까요?", {
        "유두절은 뭐하는 날인가요?",
        "유두면을 만들어 주세요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        explain_yudu(me, npc)
    else
        make_yudumyeon(me, npc)
    end
end

return {
    on_click = function(me, npc)
        local btn = me:dialog(npc, "안녕하세요? 저는 백리향입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('유두') then
            run_yudu(me, npc)
        elseif festival.is('추석') then
            run_chuseok(me, npc)
        end
    end
}
