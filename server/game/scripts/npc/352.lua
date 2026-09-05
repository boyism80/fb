-- npc: 촌장부인
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')
local function run_junghwa(me, npc)
    if not festival.is('중화절') then
        return false
    end
    local q = quest.get_annual(me, quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        return false
    end
    if me:has_items("벌레쫓는부적", 2) then
        return false
    end
    local btn = me:dialog(npc, "벌써 중화절인가요? 그럼 몽연이에게 벌레 쫓는 부적을 줘야 겠네요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "이 부적은 '향랑각시 속거천리(香娘閣氏 速去千里)'라고 써있답니다. 냄새나는 벌레를 향랑각시라고 했고, 빨리 천리를 가라는 뜻이죠", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "그럼 몽연이에게 잘 전해주세요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("벌레쫓는부적", 1)
    return true
end

local function run_seokka(me, npc)
    if not festival.is('석가탄신일') then
        return false
    end
    if me:has_items("색비단", 1) then
        me:dialog(npc, "색비단으로 예쁜 연등을 만들어 잘 쓰시길 바라겠습니다.", { prev = false, next = true })
        return true
    end
    local btn = me:dialog(npc, "주지스님이 색비단을 받아오라고 하셨군요. 드려야죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "그럼 예쁜 연등 얻으셔서 잘 쓰시길 바라겠습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("색비단", 1)
    return true
end

local function run_dano(me, npc)
    if not festival.is('단오') then
        return false
    end
    local btn = me:dialog(npc, "창포가 필요하시다구요.. 제가 모아놓은 창포를 드리도록 하죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "그냥은 못드리구요.. 하나에 100원이에요. ^^", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    local sel, list_btn = me:list(npc, "창포를 사시겠어요?", { "네. 살래요", "아니오.." }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel == 2 then
        me:dialog(npc, "흥.. 시름 말구..", { prev = false, next = true })
        return true
    end
    if sel ~= 1 then
        return true
    end
    local code = me:exchange(
        { ['money'] = 100 },
        { ['item'] = { ["창포"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "창포는 하나에 100원이에요.", { prev = false, next = true })
        return true
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 창포를 줄 수 없습니다.", { prev = false, next = false })
        return true
    end
    me:dialog(npc, "창포를 드렸습니다.", { prev = false, next = true })
    return true
end

local function run_yudu(me, npc)
    if not festival.is('유두') then
        return false
    end

    local btn = me:dialog(npc, "고사를 지내시려구요? 고사를 지내려면 유두벼, 유두콩, 유두조가 필요할텐데...", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "벼, 콩, 조를 구해오시면 제가 유두벼, 유두콩, 유두조로 바꿔드릴게요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local sel, list_btn = me:list(npc, "지금 구해오셨나요?", {
        "네, 다 구해왔어요. 지금 바꿔주세요.",
        "아니오, 아직 다 못구해왔어요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel ~= 1 then
        return true
    end

    local code = me:exchange(
        { ['item'] = { ["벼"] = 1, ["콩"] = 1, ["조"] = 1 } },
        { ['item'] = { ["유두벼"] = 1, ["유두콩"] = 1, ["유두조"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "벼, 콩, 조가 부족합니다. 다 구해오신 뒤 다시 찾아와 주세요.", { prev = false, next = false })
        return true
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 바꿔드릴 수 없습니다.", { prev = false, next = false })
        return true
    end
    me:dialog(npc, "다 됐습니다~ 그럼 경건한 마음으로 고사를 지내세요~", { prev = false, next = false })
    return true
end

local function run_chilseok(me, npc)
    if not festival.is('칠석') then
        return false
    end
    local sel, list_btn = me:list(npc, "무슨일로 오셨나요?", { "그물이 필요해요..", "식용호박이 필요해요..." }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel == nil then
        return true
    end
    if sel == 1 then
        local btn = me:dialog(npc, "그물이 필요하시다구요.. 제가 모아놓은 그물을 드리도록 하죠.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return true
        end
        btn = me:dialog(npc, "그물이 여기 있습니다. 그럼 잘 사용하세요..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return true
        end
        me:mkitem("그물", 1)
        return true
    end
    local btn = me:dialog(npc, "식용호박이 필요하시다구요.. 제가 모아놓은 식용호박을 드리도록 하죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "식용호박이 여기 있습니다. 그럼 잘 사용하세요..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("식용호박", 1)
    return true
end

local function run_chuseok(me, npc)
    if not festival.is('추석') then
        return false
    end
    local q = quest.get_annual(me, quest.QUEST_BAEKRIHYANG)
    local sel, list_btn = me:list(npc, "무슨일로 오셨나요?", { "벌초를 하고 왔어요", "식용호박이 없어서..." }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel == nil then
        return true
    end
    if sel == 1 then
        local btn = me:dialog(npc, "벌초를 하고 오셨어요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return true
        end
        if q == nil or q:step() < 1 or q:progress() < 1 then
            me:dialog(npc, "벌초를 하지 않으셨네요? 절 속이시면 안돼죠..", { prev = false, next = false })
            return true
        end
        me:dialog(npc, "수고하셨어요. 올게심니를 드릴께요.. 매달마다 특정 아이템을 하나씩 모아서 1년동안 모으시면 좋을일이 생길꺼에요. 추석엔 올게심니에요.", { prev = false, next = false })
        if me:mkitem("올게심니", 1) == nil then
            me:dialog(npc, "소지품이 가득 차서 올게심니를 줄 수 없습니다.", { prev = false, next = false })
            return true
        end
        q:progress(0)
        return true
    end
    local btn = me:dialog(npc, "식용호박이 필요하시다구요.. 제가 가지고 있는 식용호박을 드리도록 하죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("식용호박", 1)
    me:dialog(npc, "식용호박이 여기 있습니다. 그럼 잘 사용하세요..", { prev = false, next = false })
    return true
end

local function run_jungyang(me, npc)
    if not festival.is('중양절') then
        return false
    end

    local btn = me:dialog(npc, "국화가 필요하시다구요.. 제가 모아놓은 국화를 드리도록 하죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local t = datetime()
    local day_key = 0
    if t ~= nil then
        day_key = (t.year or 0) * 10000 + (t.month or 0) * 100 + (t.day or 0)
    end

    local q = quest.get_annual(me, quest.QUEST_JUNGYANG_GUKHWA)
    local given = 0
    local last_day = 0
    if q ~= nil and q:param() ~= nil and q:param() ~= "" then
        local a, b = string.match(q:param(), "^(%d+):(%d+)$")
        last_day = tonumber(a) or 0
        given = tonumber(b) or 0
        if last_day ~= day_key then
            given = 0
        end
    end

    if given >= 10 then
        me:dialog(npc, "오늘은 이미 국화를 충분히 드렸답니다. 내일 다시 오세요. (하루 10병)", { prev = false, next = false })
        return true
    end

    if me:mkitem("국화", 1) == nil then
        me:dialog(npc, "소지품이 가득 차서 국화를 줄 수 없습니다.", { prev = false, next = false })
        return true
    end

    given = given + 1
    if q == nil then
        q = me:start_quest(quest.QUEST_JUNGYANG_GUKHWA)
    end
    if q ~= nil then
        q:param(string.format("%d:%d", day_key, given))
    end

    me:dialog(npc, "국화가 여기 있습니다. 그럼 잘 사용하세요..", { prev = false, next = true })
    return true
end

local function seotdal_flags(q)
    local p = (q ~= nil and q:param()) or ''
    local y, w, d, b = string.match(p, '^(%d+):(%d+):(%d+):(%d+)$')
    if y ~= nil then
        return tonumber(w) or 0, tonumber(d) or 0, tonumber(b) or 0
    end
    w, d, b = string.match(p, '^(%d+):(%d+):(%d+)$')
    return tonumber(w) or 0, tonumber(d) or 0, tonumber(b) or 0
end

local function set_seotdal_flags(q, w, d, b)
    local year = festival.lunar_year()
    q:param(string.format('%d:%d:%d:%d', year, w, d, b))
end

local function run_seotdal(me, npc)
    if not festival.is('섣달') then
        return false
    end

    local q = quest.get_annual(me, quest.QUEST_SEOTDAL_GIFTS)
    if q == nil or q:step() < 1 or q:completed() then
        return false
    end

    local w, d, b = seotdal_flags(q)
    if w == 1 then
        me:dialog(npc, "이미 김장김치를 받았어요. 제기는 선릉이에게 잘 전해 주세요.", { prev = false, next = false })
        return true
    end

    if not me:has_items('김장김치', 1) then
        return false
    end

    local btn = me:dialog(npc, "선릉이가 김장김치를 보냈군요. 고맙습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "답례로 제기를 드릴게요. 선릉이에게 전해 주세요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local code = me:exchange(
        { ['item'] = { ['김장김치'] = 1 } },
        { ['item'] = { ['제기'] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "김장김치를 가지고 오세요.", { prev = false, next = false })
        return true
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 제기를 드릴 수 없습니다.", { prev = false, next = false })
        return true
    end

    set_seotdal_flags(q, 1, d, b)
    me:dialog(npc, "선릉이에게 제기 잘 전해 주세요.", { prev = false, next = true })
    return true
end

local function run_kimjang(me, npc)
    if not me:has_items('파김치', 1) then
        return false
    end

    local q = quest.get_annual(me, quest.QUEST_KIMJANG)
    if q ~= nil and q:completed() then
        return false
    end

    -- yeondailyy screenshots
    local btn = me:dialog(npc, "파김치를 담그셨군요! 저에게도 좀 주시면 안될까요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local sel, list_btn = me:list(npc, "파김치를 저에게 주시겠어요?", {
        "네. 드릴께요",
        "안돼요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return true
    end
    if sel ~= 1 then
        return true
    end

    local code = me:exchange(
        { ['item'] = { ['파김치'] = 1 } },
        { ['item'] = { ['고사반'] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST or code == enum.exchange_result.LACK_CAPACITY then
        return true
    end

    if q == nil then
        q = me:start_quest(quest.QUEST_KIMJANG)
    end
    if q ~= nil then
        q:complete()
        festival.mark_completed_year(q)
    end

    btn = me:dialog(npc, "어머.. 감사합니다. 그럼 저도 보답으로 뭔가 드려야 할텐데...", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "고사반을 드릴께요. 고사반은 장수를 의미하는 실타래를 숟가락에 감아서 쌀이 담아져 있는 그릇에 꽂은거에요", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:dialog(npc, "매달 아이템을 모으시고 계신가요? 고사반도 나머지 11개의 아이템과 함께 가지고 계시면 좋은일이 생길꺼에요.", { prev = false, next = true })
    return true
end

return {
    on_click = function(me, npc)
        local btn = me:dialog(npc, "저는 이 마을 촌장의 부인되는 목원지라고 합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if run_junghwa(me, npc) then
            return
        end
        if run_seokka(me, npc) then
            return
        end
        if run_dano(me, npc) then
            return
        end
        if run_yudu(me, npc) then
            return
        end
        if run_chilseok(me, npc) then
            return
        end
        if run_chuseok(me, npc) then
            return
        end
        if run_jungyang(me, npc) then
            return
        end
        if run_seotdal(me, npc) then
            return
        end
        if run_kimjang(me, npc) then
            return
        end
    end
}
