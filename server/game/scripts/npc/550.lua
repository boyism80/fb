-- npc: 마동참가신청
local quest = require('lib.quest')
local CLASS_OPTIONS = { "검신", "패왕", "마신", "신선" }
local SCHEDULE_OPTIONS = {
    { label = "1차예선 18일 / 2차예선 25일", value = "18/25" },
    { label = "1차예선 18일 / 2차예선 26일", value = "18/26" },
    { label = "1차예선 19일 / 2차예선 25일", value = "19/25" },
    { label = "1차예선 19일 / 2차예선 26일", value = "19/26" },
}

local function parse_param(p)
    if p == nil or p == "" then
        return "", "", ""
    end
    local a, b, c = p:match("^([^|]*)|([^|]*)|(.*)$")
    return a or "", b or "", c or ""
end

local function build_param(class_name, partner, schedule)
    return (class_name or "") .. "|" .. (partner or "") .. "|" .. (schedule or "")
end

local function run_form(me, npc, class_name, partner, schedule)
    local class_display = (class_name == "" and "" or class_name)
    local partner_display = (partner == "" and "" or partner)
    local schedule_display = (schedule == "" and "" or schedule)

    local labels = {
        "클래스 : " .. class_display,
        "파트너 아이디 : " .. partner_display,
        "참여일정 : " .. schedule_display,
        "제출하기",
    }
    local sel, btn = me:list(npc, "마동 최강자전 참가 신청입니다. 모든 공란을 채워 넣으신 후, 제출 버튼을 눌러주세요. 제출 뒤에는 수정이 불가능 하니 신중하게 검토하여 신청 하시기 바랍니다.", labels, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        local sub, sub_btn = me:list(npc, "마동최강자전에 참여하실 클래스를 선택하여 주세요. (파트너와 충분한 협의 후 신청하시길 바랍니다.)", CLASS_OPTIONS, { prev = false })
        if sub_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sub ~= nil and sub >= 0 and sub < #CLASS_OPTIONS then
            class_name = CLASS_OPTIONS[sub]
        end
        local q = me:quest(quest.QUEST_MADONG_APPLY)
        if q == nil then
            q = me:start_quest(quest.QUEST_MADONG_APPLY)
        end
        if q then
            q:param(build_param(class_name, partner, schedule))
        end
        run_form(me, npc, class_name, partner, schedule)
        return
    end

    if sel == 2 then
        local raw = me:input(npc, "파트너의 아이디를 입력하여 주십시오.", { top = "파트너의 아이디는", bottom = "입니다.", maxlen = 20, prev = false })
        if raw == DIALOG_RESULT.QUIT then
            return
        end
        if type(raw) == "string" and raw ~= "" then
            partner = raw
        end
        local q = me:quest(quest.QUEST_MADONG_APPLY)
        if q == nil then
            q = me:start_quest(quest.QUEST_MADONG_APPLY)
        end
        if q then
            q:param(build_param(class_name, partner, schedule))
        end
        run_form(me, npc, class_name, partner, schedule)
        return
    end

    if sel == 3 then
        local sched_labels = {}
        for i = 1, #SCHEDULE_OPTIONS do
            sched_labels[i] = SCHEDULE_OPTIONS[i].label
        end
        local sub, sub_btn = me:list(npc, "참여하실 예선전의 일정을 선택 해 주세요.", sched_labels, { prev = false })
        if sub_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sub ~= nil and sub >= 0 and sub < #SCHEDULE_OPTIONS then
            schedule = SCHEDULE_OPTIONS[sub].value
        end
        local q = me:quest(quest.QUEST_MADONG_APPLY)
        if q == nil then
            q = me:start_quest(quest.QUEST_MADONG_APPLY)
        end
        if q then
            q:param(build_param(class_name, partner, schedule))
        end
        run_form(me, npc, class_name, partner, schedule)
        return
    end

    if sel == 4 then
        if class_name == "" or partner == "" or schedule == "" then
            me:dialog(npc, "클래스, 파트너 아이디, 참여일정을 모두 입력해 주세요.", { prev = false, next = false })
            run_form(me, npc, class_name, partner, schedule)
            return
        end
        local confirm, confirm_btn = me:list(npc, "정말 이대로 제출하시겠습니까?", { "예.", "아니오." }, { prev = false })
        if confirm_btn == DIALOG_RESULT.QUIT then
            return
        end
        if confirm ~= 1 then
            run_form(me, npc, class_name, partner, schedule)
            return
        end
        local q = me:quest(quest.QUEST_MADONG_APPLY)
        if q == nil then
            q = me:start_quest(quest.QUEST_MADONG_APPLY)
            if q == nil then
                me:dialog(npc, "제출에 실패했습니다.", { prev = false, next = false })
                return
            end
        end
        if q then
            q:step(1)
            q:param(build_param(class_name, partner, schedule))
        end
        me:dialog(npc, "참가 신청이 접수되었습니다. (서버 로그 기록은 지원되지 않습니다.)", { prev = false, next = false })
        return
    end
end

return {
    on_click = function(me, npc)
        local q = me:quest(quest.QUEST_MADONG_APPLY)
        if q ~= nil and q:step() == 1 then
            local class_name, partner, schedule = parse_param(q:param())
            me:dialog(npc, "이미 제출하셨습니다.\n선택클래스:" .. class_name .. "\n파트너아이디:" .. partner .. "\n참여일:" .. schedule .. "", { prev = false, next = false })
            return
        end

        if me:promotion() < 3 then
            me:dialog(npc, "3차승급 미만은 참여하실 수 없습니다.", { prev = false, next = false })
            return
        end

        local d = me:dialog(npc, "약 1주일간 참가신청을 받은 뒤, 참가 자격 필터링을 한 뒤 최종 참가자를 발표 합니다.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        d = me:dialog(npc, "참가팀은 총 64팀이며, 자진기권 등의 가능성을 염두에 두어 예비 팀도 약 10팀을 선발합니다.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        d = me:dialog(npc, "2인 1조로 참여하실 수 있으며, 함께 참가하실 파트너와 함께 참가 등록을 하셔야만 합니다.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        d = me:dialog(npc, "참가 과정에서 별도의 파트너 동의 여부는 이루어지지 않습니다만, 추 후 검토하게 되므로 반드시 신중하게 참가 신청을 해 주시길 바랍니다.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end

        local class_name, partner, schedule = "", "", ""
        if q ~= nil then
            class_name, partner, schedule = parse_param(q:param())
        end
        run_form(me, npc, class_name, partner, schedule)
    end
}
