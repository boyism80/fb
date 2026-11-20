local lib      = require("integration.lib")
local protocol = require("integration.protocol")
local qd       = require("integration.lib.quest_dialog")

local QUEST_3RD = "230"
local QUEST_4TH = "231"
local HEAVEN    = 621
local SCALES    = 654
local SHRINE    = 151
local SAGE      = 1748
local MUTA      = 4001
local EAST_ROOM = 1772
local WEST_ROOM = 1773
local STORM     = 1774
local CAVE      = 1758
local DARK      = 1776
local MAZE      = 1760
local CAVE8     = 1757
local WAITING   = 1775
local GUILLIM   = 1763

local KEYS = {
    {
        map    = "길림성",
        id     = GUILLIM,
        x      = 213,
        y      = 106,
        mate_x = 214,
        mate_y = 106,
        dir    = "TOP",
        dest   = EAST_ROOM,
        key    = "동문열쇠",
    },
    {
        map    = "길림성",
        id     = GUILLIM,
        x      = 26,
        y      = 174,
        mate_x = 27,
        mate_y = 174,
        dir    = "TOP",
        dest   = WEST_ROOM,
        key    = "서문열쇠",
    },
}

local BOSSES = {
    {
        map    = "장안성신전9",
        id     = 1976,
        x      = 16,
        y      = 1,
        mate_x = 17,
        mate_y = 1,
        dir    = "TOP",
        dest   = 2009,
        token  = "소호의증표",
        opened = "소호를 잘",
        after  = "2",
    },
    {
        map    = "국내성신전9",
        id     = 1985,
        x      = 16,
        y      = 38,
        mate_x = 17,
        mate_y = 38,
        dir    = "BOTTOM",
        dest   = 2008,
        token  = "전욱의증표",
        opened = "전욱까지",
        after  = "3",
    },
    {
        map    = "부여성신전9",
        id     = 1961,
        x      = 38,
        y      = 6,
        mate_x = 38,
        mate_y = 7,
        dir    = "RIGHT",
        dest   = 1964,
        token  = "제곡의증표",
        opened = "제곡을 물리치",
        after  = "4",
    },
    {
        map    = "일본신전9",
        id     = 1995,
        x      = 1,
        y      = 6,
        mate_x = 1,
        mate_y = 7,
        dir    = "LEFT",
        dest   = 2007,
        token  = "요제의증표",
        opened = "요제까지",
        after  = "5",
    },
    {
        map    = "광동성신전입구",
        id     = 1967,
        x      = 10,
        y      = 9,
        mate_x = 10,
        mate_y = 10,
        dir    = "LEFT",
        dest   = 2006,
        token  = "순제의증표",
        opened = "오제를 모두",
        after  = "6",
        sunje  = true,
    },
    {
        map    = "순제의방",
        id     = 2006,
        x      = 14,
        y      = 5,
        mate_x = 15,
        mate_y = 5,
        dir    = "TOP",
        dest   = 2010,
        token  = "반고의심장",
        opened = "다시 보게 되어",
        after  = "7",
        heart  = true,
    },
}

local grouped = true

local function fail(bot, label)
    log("fatal", string.format("promotion %s: %s", bot:name(), label))
    return false
end

local function has_text(text)
    return function(packet)
        return packet.message ~= nil and packet.message:find(text, 1, true) ~= nil
    end
end

local function is_list(text)
    return function(packet)
        return packet.type == "list" and packet.message ~= nil and packet.message:find(text, 1, true) ~= nil
    end
end

local function prepare(bot, class_name, hp, step, promotion)
    bot:chat("/직업바꾸기 " .. class_name)
    bot:chat("/레벨바꾸기 99")
    bot:chat("/체력바꾸기 " .. tostring(hp))
    if qd.check(bot, "verify_promotion", step, tostring(promotion)) ~= true then
        return fail(bot, step)
    end
    return true
end

local function give(bot, name, step)
    bot:chat("/아이템생성 " .. name .. " 1")
    if qd.check(bot, "verify_items", step, name .. "=1") ~= true then
        return fail(bot, step)
    end
    return true
end

local function give_all(bot, names, step)
    local spec = {}
    for _, name in ipairs(names) do
        bot:chat("/아이템생성 " .. name .. " 1")
        spec[#spec + 1] = name .. "=1"
    end
    if qd.check(bot, "verify_items", step, table.concat(spec, ",")) ~= true then
        return fail(bot, step)
    end
    return true
end

local function spot(bot)
    local pos = bot:position()
    local _, dialog = bot:last_dialog()
    local text = ""
    if dialog ~= nil then
        text = string.format(" type=%s message=%s", tostring(dialog.type), tostring(dialog.message))
    end
    if pos == nil then
        return string.format("map=%s pos=?%s", tostring(bot:map()), text)
    end
    return string.format("map=%s pos=%s,%s%s", tostring(bot:map()), tostring(pos[1]), tostring(pos[2]), text)
end

local function arrive(bot, map_name, x, y, map_id, label)
    bot:map_move(map_name, x, y)
    local waited = 0
    while waited < 5000 do
        local pos = bot:position()
        if bot:map() == map_id and pos ~= nil and pos[1] == x and pos[2] == y then
            return true
        end
        bot:sleep(100)
        waited = waited + 100
    end
    return fail(bot, string.format("%s %s", label, spot(bot)))
end

local function await_dialog(bot, seq, pred, timeout)
    local waited = 0
    while waited < timeout do
        local current, dialog = bot:last_dialog()
        if current ~= seq and dialog ~= nil and pred(dialog) then
            return dialog
        end
        bot:sleep(200)
        waited = waited + 200
    end
    return nil
end

local function wait_at(ctx, bot, map_id, x, y, timeout)
    local waited = 0
    while waited < timeout do
        local pos = bot:position()
        if bot:map() == map_id and pos ~= nil and pos[1] == x and pos[2] == y then
            return true
        end
        ctx:sleep(200)
        waited = waited + 200
    end
    return false
end

local function wait_both(ctx, bot, mate, dest, label)
    local waited = 0
    while waited < 15000 do
        if bot:map() == dest and mate:map() == dest then
            return true
        end
        ctx:sleep(200)
        waited = waited + 200
    end
    return fail(bot,
        string.format("%s leader=%s mate=%s expected=%s", label, tostring(bot:map()), tostring(mate:map()),
            tostring(dest)))
end

local function chain(bot, snippets, label)
    for _, text in ipairs(snippets) do
        if qd.next_dialog(bot, has_text(text)) == nil then
            return fail(bot, label .. " " .. text)
        end
    end
    qd.press(bot, "NEXT")
    return true
end

local function walk_into(bot, map_name, x, y, map_id, direction, dest, label)
    if arrive(bot, map_name, x, y, map_id, label) ~= true then
        return false
    end
    bot:move(direction)
    local waited = 0
    while waited < 15000 do
        if bot:map() == dest then
            return true
        end
        bot:sleep(200)
        waited = waited + 200
    end
    return fail(bot, label .. " dest=" .. tostring(bot:map()))
end

local function offer(bot, index, tails)
    if arrive(bot, "길림청룡신전", 15, 8, SHRINE, "shrine") ~= true then
        return false
    end
    local seq = bot:last_dialog()
    bot:move("TOP")
    if await_dialog(bot, seq, has_text("무슨일로"), 8000) == nil then
        return fail(bot, "shrine greeting " .. spot(bot))
    end
    if bot:request_dialog_ext(protocol.dialog("LIST", 0, "", 2, 0, "", "NEXT"), has_text("무엇을 제물로")) == nil then
        return fail(bot, "shrine offer")
    end
    if bot:request_dialog_ext(protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"), has_text("선물을 주겠")) == nil then
        return fail(bot, "shrine gift " .. tostring(index))
    end
    return chain(bot, tails, "shrine")
end

local function enter_key(ctx, bot, mate, door)
    if wait_at(ctx, mate, door.id, door.mate_x, door.mate_y, 180000) ~= true then
        return fail(bot, "mate " .. door.key)
    end
    if arrive(bot, door.map, door.x, door.y, door.id, door.key) ~= true then
        return false
    end
    if wait_at(ctx, mate, door.id, door.mate_x, door.mate_y, 5000) ~= true then
        return fail(bot, "mate left " .. door.key)
    end
    bot:move(door.dir)
    return wait_both(ctx, bot, mate, door.dest, door.key)
end

local function enter_boss(ctx, bot, mate, door)
    if wait_at(ctx, mate, door.id, door.mate_x, door.mate_y, 180000) ~= true then
        return fail(bot, "mate " .. door.map)
    end
    if arrive(bot, door.map, door.x, door.y, door.id, door.map) ~= true then
        return false
    end
    if wait_at(ctx, mate, door.id, door.mate_x, door.mate_y, 5000) ~= true then
        return fail(bot, "mate left " .. door.map)
    end
    local seq = bot:last_dialog()
    bot:move(door.dir)
    if await_dialog(bot, seq, has_text("동료가 있느냐"), 8000) == nil then
        return fail(bot, "door " .. door.map .. " " .. spot(bot))
    end
    if bot:request_dialog_ext(protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"), function(packet)
            return packet.type == "input_ext"
        end) == nil then
        return fail(bot, "name " .. door.map)
    end
    if bot:request_dialog_ext(protocol.dialog("INPUT_EXT", 2, mate:name(), 0, 0, "", "NEXT"), is_list("또다른 동료")) == nil then
        return fail(bot, "confirm " .. door.map .. " " .. spot(bot))
    end
    bot:send(protocol.dialog("LIST", 0, "", 2, 0, "", "NEXT"))
    return wait_both(ctx, bot, mate, door.dest, door.map)
end

local function ride_keys(ctx, slot)
    local bot = ctx:bot(slot)
    for _, door in ipairs(KEYS) do
        bot:map_move(door.map, door.mate_x, door.mate_y)
        local waited = 0
        local arrived = false
        while waited < 180000 do
            if bot:map() == door.dest then
                arrived = true
                bot:sleep(3000)
                break
            end
            bot:sleep(200)
            waited = waited + 200
        end
        if arrived ~= true then
            return fail(bot, "ride " .. door.key)
        end
    end
    return true
end

local function ride_bosses(ctx, slot)
    local bot = ctx:bot(slot)
    if #bot:name() > 18 then
        return fail(bot, "name longer than 18")
    end
    for _, door in ipairs(BOSSES) do
        local seq = bot:last_dialog()
        bot:map_move(door.map, door.mate_x, door.mate_y)
        local waited = 0
        local arrived = false
        local answered = false
        while waited < 180000 do
            if bot:map() == door.dest then
                arrived = true
                bot:sleep(3000)
                break
            end
            local current, dialog = bot:last_dialog()
            if answered ~= true and current ~= seq and dialog ~= nil and dialog.message ~= nil and dialog.message:find("같이 들어가겠", 1, true) ~= nil then
                bot:send(protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"))
                answered = true
                seq = current
            end
            bot:sleep(200)
            waited = waited + 200
        end
        if arrived ~= true or answered ~= true then
            return fail(bot, "ride " .. door.map)
        end
    end
    return true
end

local function promote_class(bot, option, step, expected)
    if qd.open_pursuit(bot, "선구륜", option, has_text("어서 오세요")) == nil then
        return fail(bot, step .. " greet")
    end
    if bot:request_dialog(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"), function(packet)
            return packet.type == "pursuit"
        end) == nil then
        return fail(bot, step .. " ask")
    end
    if bot:request_dialog_ext(protocol.dialog("PURSUIT", 0, "", 0, 0, "네, 모든 준비가 끝났습니다."), has_text("승급을 마쳤습니다")) == nil then
        return fail(bot, step .. " done")
    end
    qd.press(bot, "NEXT")
    if qd.check(bot, "verify_promotion", step, tostring(expected)) ~= true then
        return fail(bot, step)
    end
    return true
end

local function rank1(ctx)
    local bot = ctx:bot(0)
    if grouped ~= true then
        return fail(bot, "group")
    end
    if prepare(bot, "전사", 70000, "rank1_start", 0) ~= true then
        return false
    end
    if give_all(bot, { "건괘", "곤괘", "진괘", "감괘", "리괘", "태괘", "선괘", "간괘" }, "trigrams") ~= true then
        return false
    end
    if arrive(bot, "천상계제일계", 10, 10, HEAVEN, "heaven") ~= true then
        return false
    end
    if qd.open_pursuit(bot, "랑구륜", "팔괘", has_text("어서 오십시요")) == nil then
        return fail(bot, "palgu greet")
    end
    if qd.next_dialog(bot, has_text("8개의 괘를 팔괘로")) == nil then
        return fail(bot, "palgu explain")
    end
    local listed = qd.next_dialog(bot, is_list("바꾸시겠어요"))
    if qd.select_option(bot, listed, "네", has_text("다 가져오셨군요")) == nil then
        return fail(bot, "palgu accept")
    end
    if qd.next_dialog(bot, has_text("만들어 드렸습니다")) == nil then
        return fail(bot, "palgu made")
    end
    qd.press(bot, "NEXT")
    if qd.check(bot, "verify_items", "palgu", "팔괘=1") ~= true then
        return fail(bot, "palgu")
    end
    return promote_class(bot, "전사로서 승급을 원합니다", "rank1", 1)
end

local function rank2(ctx)
    local bot = ctx:bot(1)
    if grouped ~= true then
        return fail(bot, "group")
    end
    if prepare(bot, "검객", 200000, "rank2_start", 1) ~= true then
        return false
    end
    if arrive(bot, "수룡,화룡의방", 10, 10, SCALES, "scales") ~= true then
        return false
    end
    if give_all(bot, { "수룡의비늘", "화룡의비늘" }, "scales_item") ~= true then
        return false
    end
    if arrive(bot, "천상계제일계", 12, 12, HEAVEN, "heaven2") ~= true then
        return false
    end
    return promote_class(bot, "검객으로서 승급을 원합니다", "rank2", 2)
end

local function rank3(ctx)
    local bot = ctx:bot(2)
    local mate = ctx:bot(3)
    if grouped ~= true then
        return fail(bot, "group")
    end
    if prepare(bot, "검제", 600000, "rank3_start", 2) ~= true then
        return false
    end
    if arrive(bot, "천성현의방", 10, 10, SAGE, "sage") ~= true then
        return false
    end
    local asked = qd.open_pursuit(bot, "천성현", "3차 승급을 원합니다", is_list("이야기를 좀 들어"))
    if asked == nil then
        return fail(bot, "sage pursuit")
    end
    if qd.select_option(bot, asked, "네, 이야기 하세요.", has_text("길림성이란")) == nil then
        return fail(bot, "sage listen")
    end
    if qd.next_dialog(bot, has_text("신전이 각 방위")) == nil then
        return fail(bot, "sage temples")
    end
    if qd.next_dialog(bot, has_text("힘이 예전보다")) == nil then
        return fail(bot, "sage weak")
    end
    if qd.next_dialog(bot, has_text("동물들도")) == nil then
        return fail(bot, "sage beasts")
    end
    local survey = qd.next_dialog(bot, is_list("조사해"))
    if qd.select_option(bot, survey, "네, 제가 조사하겠습니다.", has_text("증표를 드리겠습니다")) == nil then
        return fail(bot, "sage accept")
    end
    if chain(bot, { "잃어버리셔선", "제단을 찾아가", "현무의 제단", "성공하시길" }, "sage send") ~= true then
        return false
    end
    if qd.check(bot, "verify_quest", "sage_started", QUEST_3RD, "1") ~= true then
        return fail(bot, "sage quest")
    end
    if qd.check(bot, "verify_items", "sage_letter", "승급의서장=1") ~= true then
        return fail(bot, "sage letter")
    end
    qd.remove_npc(bot)

    if give(bot, "물의옥", "water") ~= true then
        return false
    end
    if offer(bot, 4, { "동쪽에 있는 태극문", "이 열쇠를 가지고", "또 다른 암흑" }) ~= true then
        return false
    end
    if qd.check(bot, "verify_items", "east_key", "동문열쇠=1") ~= true then
        return fail(bot, "east key")
    end
    if enter_key(ctx, bot, mate, KEYS[1]) ~= true then
        return false
    end

    if give(bot, "불의옥", "fire") ~= true then
        return false
    end
    if offer(bot, 3, { "첫번째 권", "두번째 책", "북쪽의 미로" }) ~= true then
        return false
    end
    if give(bot, "구름의옥", "cloud") ~= true then
        return false
    end
    if offer(bot, 2, { "서쪽에 있는 태극문", "마지막 암흑왕" }) ~= true then
        return false
    end
    if qd.check(bot, "verify_items", "west_key", "서문열쇠=1,고대마법서'상=1") ~= true then
        return fail(bot, "west key")
    end
    if enter_key(ctx, bot, mate, KEYS[2]) ~= true then
        return false
    end

    if give(bot, "산의옥", "mountain") ~= true then
        return false
    end
    if offer(bot, 1, { "마법서 두권", "성현을 찾아", "힘을 주고" }) ~= true then
        return false
    end
    if qd.check(bot, "verify_items", "books", "고대마법서'상=1,고대마법서'하=1") ~= true then
        return fail(bot, "books")
    end
    if walk_into(bot, "길림미로2", 16, 14, MAZE, "TOP", STORM, "storm") ~= true then
        return false
    end
    if walk_into(bot, "길림동굴8", 20, 2, CAVE8, "TOP", CAVE, "cave") ~= true then
        return false
    end
    if walk_into(bot, "암흑왕대기실", 9, 22, WAITING, "BOTTOM", DARK, "dark") ~= true then
        return false
    end
    if give(bot, "암흑왕의봉인", "seal") ~= true then
        return false
    end

    if arrive(bot, "천성현의방", 10, 10, SAGE, "sage return") ~= true then
        return false
    end
    if qd.open_pursuit(bot, "천성현", "3차 승급을 원합니다", has_text("신수의 부탁")) == nil then
        return fail(bot, "sage books")
    end
    if chain(bot, {
            "아직 이 일이 끝난",
            "암흑의 존재에서",
            "다시금 부활",
            "마법책을 읽어",
            "동쪽과 북쪽",
            "서쪽의 어둠",
            "암흑왕이 숨어",
            "축복해드리",
            "바로 저에게로",
            "부활해 버립니다",
            "행운을 빕니다",
            "성공하셨",
            "증표를 가지고",
            "선녀들에게로",
        }, "sage finish") ~= true then
        return false
    end
    if qd.check(bot, "verify_items", "token3", "천성현의증표=1") ~= true then
        return fail(bot, "token3")
    end
    if qd.check(bot, "verify_quest", "sage_done", QUEST_3RD, "2:done") ~= true then
        return fail(bot, "sage done")
    end
    qd.remove_npc(bot)
    if arrive(bot, "천상계제일계", 8, 8, HEAVEN, "heaven3") ~= true then
        return false
    end
    return promote_class(bot, "검제로서 승급을 원합니다", "rank3", 3)
end

local function turn_in(bot, door)
    if give(bot, door.token, door.token) ~= true then
        return false
    end
    if arrive(bot, "무타의집", 8, 8, MUTA, "muta " .. door.token) ~= true then
        return false
    end
    if qd.click(bot, "무타", has_text(door.opened)) == nil then
        return fail(bot, "muta " .. door.token)
    end
    if door.sunje == true then
        if qd.next_dialog(bot, has_text("제물이었")) == nil then
            return fail(bot, "sunje tale")
        end
        local choice = qd.next_dialog(bot, is_list("어떻게 하겠나"))
        if qd.select_option(bot, choice, "이 땅이 아직 건재한데, 포기하긴 이릅니다.", has_text("기개가 대단")) == nil then
            return fail(bot, "sunje choice")
        end
        if chain(bot, { "곧장 나에게" }, "sunje send") ~= true then
            return false
        end
    elseif door.heart == true then
        if chain(bot, { "천성랑을 찾아" }, "heart send") ~= true then
            return false
        end
        if qd.check(bot, "verify_items", "heart_kept", "반고의심장=1") ~= true then
            return fail(bot, "heart kept")
        end
    else
        if chain(bot, { "곧장 나에게" }, door.token) ~= true then
            return false
        end
    end
    if qd.check(bot, "verify_quest", "muta_" .. door.after, QUEST_4TH, door.after) ~= true then
        return fail(bot, "muta step " .. door.after)
    end
    qd.remove_npc(bot)
    return true
end

local function rank4(ctx)
    local bot = ctx:bot(4)
    local mate = ctx:bot(5)
    if grouped ~= true then
        return fail(bot, "group")
    end
    if #mate:name() > 18 then
        return fail(bot, "mate name longer than 18")
    end
    if prepare(bot, "검황", 1400000, "rank4_start", 3) ~= true then
        return false
    end
    if arrive(bot, "천성현의방", 8, 8, SAGE, "cheonsung") ~= true then
        return false
    end
    if qd.open_pursuit(bot, "천성랑", "4차 승급을 원합니다", has_text("무타")) == nil then
        return fail(bot, "cheonsung start")
    end
    qd.press(bot, "NEXT")
    if qd.check(bot, "verify_quest", "muta_started", QUEST_4TH, "0") ~= true then
        return fail(bot, "muta start")
    end
    qd.remove_npc(bot)
    if arrive(bot, "무타의집", 8, 8, MUTA, "muta intro") ~= true then
        return false
    end
    if qd.click(bot, "무타", has_text("심상치 않은 기운")) == nil then
        return fail(bot, "muta intro")
    end
    if qd.next_dialog(bot, has_text("천기를 살펴본")) == nil then
        return fail(bot, "muta sky")
    end
    if qd.next_dialog(bot, has_text("반고의 부활로")) == nil then
        return fail(bot, "muta doom")
    end
    local vow = qd.next_dialog(bot, is_list("정녕 이대로"))
    if qd.select_option(bot, vow, "제가 오제의 음모를 막아보겠습니다.", has_text("그래보겠는가")) == nil then
        return fail(bot, "muta vow")
    end
    if chain(bot, { "한단계 더 높은", "제물을 찾아내기", "오제란 소호", "소호를 물리치고" }, "muta send") ~= true then
        return false
    end
    if qd.check(bot, "verify_quest", "muta_hunt", QUEST_4TH, "1") ~= true then
        return fail(bot, "muta hunt")
    end
    qd.remove_npc(bot)

    for _, door in ipairs(BOSSES) do
        if enter_boss(ctx, bot, mate, door) ~= true then
            return false
        end
        if turn_in(bot, door) ~= true then
            return false
        end
    end

    if arrive(bot, "천성현의방", 8, 8, SAGE, "cheonsung return") ~= true then
        return false
    end
    local ready = qd.open_pursuit(bot, "천성랑", "4차 승급을 원합니다", is_list("승급을 원하시나요"))
    if qd.select_option(bot, ready, "예", has_text("승급을 마쳤습니다")) == nil then
        return fail(bot, "rank4 done")
    end
    qd.press(bot, "NEXT")
    if qd.check(bot, "verify_promotion", "rank4", "4") ~= true then
        return fail(bot, "rank4")
    end
    if qd.check(bot, "verify_quest", "muta_done", QUEST_4TH, "7:done") ~= true then
        return fail(bot, "muta done")
    end
    return true
end

test_suite {
    name          = "Promotion Test",
    bot_count     = 6,
    should_skip   = function(_ctx)
        if localhost() then
            log("debug", "Promotion test skipped on local host (requires server transfer)")
            return true
        end
        return false
    end,
    on_initialize = function(ctx)
        if lib.group.form(ctx) == false then
            grouped = false
        end
    end,
    scenarios     = {
        {
            parallel = {
                [0] = { rank1 },
                [1] = { rank2 },
                [2] = { rank3 },
                [3] = { function(ctx) return ride_keys(ctx, 3) end },
                [4] = { rank4 },
                [5] = { function(ctx) return ride_bosses(ctx, 5) end },
            },
        },
    },
}
