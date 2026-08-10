-- npc: 백나연
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

-- Achievement title from yeondailyy screenshot: 김장철에 김치를 담가보자
local ACHIEVEMENT_KIMJANG = 563
local FERMENT_SEC = 2 * 86400

-- Menu order (1-based) matches deposit/pickup lists.
-- Param: {year}:ferment|{id}:{ready_ts}|{id}:{ready_ts}|...
-- Legacy single-slot `{year}:ferment|{ready_ts}` migrates to 파김치 (id 4).
local KIMCHI_MENU = {
    "배추김치",
    "총각김치",
    "동치미",
    "파김치",
    "열무김치",
}

-- Entries without materials are not depositable yet (pickup works once slotted).
local KIMCHI = {
    [1] = {
        name = "배추김치",
        item = "배추김치",
        -- TODO(2026-음력10월 / 김장절): materials + material_dialog
    },
    [2] = {
        name = "총각김치",
        item = "총각김치",
        -- TODO(2026-음력10월 / 김장절): materials + material_dialog
    },
    [3] = {
        name = "동치미",
        item = "동치미",
        -- TODO(2026-음력10월 / 김장절): materials + material_dialog
    },
    [4] = {
        name = "파김치",
        item = "파김치",
        materials = { ["파"] = 1, ["고춧가루"] = 1, ["액젓"] = 1, ["마늘"] = 1, ["소금"] = 1 },
        material_dialog = "파김치에 필요한 재료는 파 고추가루 액젓 소금 마늘 입니다.",
    },
    [5] = {
        name = "열무김치",
        item = "열무김치",
        -- TODO(2026-음력10월 / 김장절): materials + material_dialog
    },
}

local function parse_ferment_slots(q)
    local slots = {}
    if q == nil then
        return slots
    end

    local p = q:param() or ''
    local body = string.match(p, '^%d+:ferment|(.*)$')
    if body == nil or body == '' then
        return slots
    end

    -- Legacy: one timestamp, no type id → 파김치
    if not string.find(body, ':', 1, true) then
        local ts = tonumber(body)
        if ts ~= nil then
            slots[4] = ts
        end
        return slots
    end

    for id, ts in string.gmatch(body, '(%d+):(%d+)') do
        slots[tonumber(id)] = tonumber(ts)
    end
    return slots
end

local function write_ferment_slots(q, slots)
    local ids = {}
    for id in pairs(slots) do
        ids[#ids + 1] = id
    end
    table.sort(ids)

    if #ids == 0 then
        q:param(tostring(festival.lunar_year()))
        return
    end

    local parts = {}
    for i = 1, #ids do
        local id = ids[i]
        parts[i] = string.format('%d:%d', id, slots[id])
    end
    q:param(string.format('%d:ferment|%s', festival.lunar_year(), table.concat(parts, '|')))
end

local function set_ferment_slot(q, kimchi_id, ready_ts)
    local slots = parse_ferment_slots(q)
    slots[kimchi_id] = ready_ts
    write_ferment_slots(q, slots)
end

local function clear_ferment_slot(q, kimchi_id)
    local slots = parse_ferment_slots(q)
    slots[kimchi_id] = nil
    write_ferment_slots(q, slots)
end

local function ensure_kimjang_quest(me)
    local q = quest.get_annual(me, quest.QUEST_KIMJANG)
    if q == nil then
        q = me:start_quest(quest.QUEST_KIMJANG)
        if q ~= nil then
            festival.stamp_lunar_year(q)
        end
    end
    return q
end

local function has_all_materials(me, materials)
    for name, count in pairs(materials) do
        if not me:has_items(name, count) then
            return false
        end
    end
    return true
end

local function run_junghwa(me, npc)
    if not festival.is('중화절') then
        return false
    end

    local q = quest.get_annual(me, quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        return false
    end

    if me:has_items("깨진김장독", 1) then
        return false
    end

    local button = me:dialog(npc, "몽연오빠가 청소한다구요? 잘됐다~~ 김장독 깨진게 있어서 어떻게할지 고민하고 있었는데.", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return true
    end
    button = me:dialog(npc, "깨진거니까 다치지 않도록 조심해서 가져가세요! 감사합니다!!", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("깨진김장독", 1)
    return true
end

local function run_pickup(me, npc, q)
    local btn = me:dialog(npc, "김치를 찾으러 오셨다구요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "어떤 김치를 맡기셨나요?", KIMCHI_MENU, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    local kimchi = KIMCHI[sel]
    if kimchi == nil then
        return
    end

    btn = me:dialog(npc, string.format("%s를 맡기셨다구요?\n어디보자...", kimchi.name), { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local slots = parse_ferment_slots(q)
    local ready = slots[sel]
    if ready == nil then
        return
    end

    if now() < ready then
        me:dialog(npc, string.format("모레 저에게 다시 오시면 제가 잘 익은 %s를 드리겠습니다.", kimchi.name), { prev = false, next = true })
        return
    end

    if me:mkitem(kimchi.item, 1) == nil then
        return
    end

    clear_ferment_slot(q, sel)
    me:push_achievement(ACHIEVEMENT_KIMJANG, "김장철에 김치를 담가보자", 7, 1)
    me:dialog(npc, "김치가 맛있게 익었네요~\n맛있게드세요!", { prev = false, next = true })
    -- Post-pickup tip from yeondailyy screenshot (not in Desktop/김장철 set).
    me:dialog(npc, "김장집을 나가 개성댁을 찾아가보세요", { prev = false, next = true })
end

-- TODO(2026-음력10월 / 김장절): lore options 1–2 full NPC pages not fully transcribed here.
local function run_deposit(me, npc, q)
    local btn = me:dialog(npc, "어머? 김장 재료를 받아오셨어요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "어떤 김치를 담그실건가요?", KIMCHI_MENU, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    local kimchi = KIMCHI[sel]
    if kimchi == nil or kimchi.materials == nil then
        -- TODO(2026-음력10월 / 김장절): other kimchi types
        return
    end

    local slots = parse_ferment_slots(q)
    if slots[sel] ~= nil then
        me:dialog(npc, string.format("이미 저에게 %s를 부탁하셨는데요?", kimchi.name), { prev = false, next = true })
        return
    end

    btn = me:dialog(npc, kimchi.material_dialog, { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local confirm, confirm_btn = me:list(npc, "재료를 다 구해 오셨나요?", {
        "네, 다 구해왔어요.",
        "아니오, 재료가 모자라네요..",
    }, { prev = false })
    if confirm_btn == DIALOG_RESULT.QUIT or confirm == nil then
        return
    end

    if confirm ~= 1 then
        return
    end

    btn = me:dialog(npc, "그럼 재료가 다 있는지 확인해볼께요..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    if not has_all_materials(me, kimchi.materials) then
        return
    end

    local code = me:exchange({ ['item'] = kimchi.materials }, nil)
    if code == enum.exchange_result.LACK_COST then
        return
    end

    btn = me:dialog(npc, string.format("재료를 다 가지고 오셨으니 %s를 담아드릴께요.", kimchi.name), { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "자.. 그럼 재료가 다\n준비되었으니.. 김치를 다 만들고\n이틀정도만 기다리면 되겠네요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    me:dialog(npc, string.format("모레 저에게 다시 오시면 제가 잘 익은 %s를 드리겠습니다.", kimchi.name), { prev = false, next = true })
    set_ferment_slot(q, sel, now() + FERMENT_SEC)
end

return {
    on_click = function(me, npc)
        if not festival.is('김장') and not festival.is('중화절') then
            return
        end

        if festival.is('김장') then
            local button = me:dialog(npc, "안녕하세요!\n맛있는 김치를 담가요!", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end

            local q = ensure_kimjang_quest(me)
            if q == nil then
                return
            end

            local menu_sel, menu_btn = me:list(npc, "제가 도와드릴 일이 있나요?", {
                "김장이 뭐에요?",
                "김치의 종류엔 어떤것들이 있나요?",
                "김치를 담아주세요!",
                "김치를 찾으러 왔어요..",
            }, { prev = false })
            if menu_btn == DIALOG_RESULT.QUIT or menu_sel == nil then
                return
            end

            if menu_sel == 1 or menu_sel == 2 then
                -- TODO: lore pages not fully transcribed yet
                return
            elseif menu_sel == 3 then
                run_deposit(me, npc, q)
                return
            elseif menu_sel == 4 then
                run_pickup(me, npc, q)
                return
            end
            return
        end

        local button = me:dialog(npc, "안녕하세요? 저는 백나연입니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if run_junghwa(me, npc) then
            return
        end
    end
}
