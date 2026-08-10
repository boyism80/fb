-- npc: 백남신
local festival = require('lib.festival')
local enum = require('lib.enum')
local quest = require('lib.quest')

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
    local q = quest.get_annual(me, quest.QUEST_SEOTDAL_GIFTS)
    if q == nil or q:step() < 1 or q:completed() then
        return false
    end

    local w, d, b = seotdal_flags(q)
    if b == 1 or me:has_items('설화의이슬', 1) then
        me:dialog(npc, "이미 설화의이슬을 드렸지요. 어른들 몰래 드세요.", { prev = false, next = false })
        return true
    end

    if not me:has_items('말린생선', 1) then
        return false
    end

    local btn = me:dialog(npc, "선릉이가 말린생선을 보냈군요. 고맙습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "눈빛에서 피는 꽃잎에 생기는 이슬을 모아 만든 술입니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "설화의이슬을 어른들 몰래 드릴게요. 맛있게 드세요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local code = me:exchange(
        { ['item'] = { ['말린생선'] = 1 } },
        { ['item'] = { ['설화의이슬'] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "말린생선을 가지고 오세요.", { prev = false, next = false })
        return true
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 설화의이슬을 드릴 수 없습니다.", { prev = false, next = false })
        return true
    end

    set_seotdal_flags(q, w, d, 1)
    return true
end

local function run_jungyang(me, npc)
    local btn = me:dialog(npc, "안녕하세요? 중양절입니다..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "음력 9월 9일은 중양절입니다. 중양절은 다른 말로 '중구일'이라고도 하지요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "옛 어른들은 홀수가 두 번 겹치면 복이 들어오는 좋은 날이라고 단오나 칠석날처럼 중양절을 명절로 삼았지요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "중양절에는 높은 곳에 올라가 국화로 빚은 술을 마시며 즐겁게 놀거나 술친구를 찾아가 함께 놀거나 술을 선물하기도 했지요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "촌장집에서 국화를 구해 오시면 제가 국화주를 담아드리겠습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    if not me:has_items('국화', 1) then
        return
    end

    btn = me:dialog(npc, "국화를 구해오셨나요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "잠시만 기다려 주세요.....", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ['국화'] = 1 } },
        { ['item'] = { ['국화주'] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "국화가 부족합니다.", { prev = false, next = false })
        return
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 국화주를 드릴 수 없습니다.", { prev = false, next = false })
        return
    end

    me:dialog(npc, "그럼 맛있게 드세요.", { prev = false, next = true })
end

return {
    on_click = function(me, npc)
        local btn = me:dialog(npc, "안녕하세요? 저는 백남신입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('섣달') then
            if run_seotdal(me, npc) then
                return
            end
            me:dialog(npc, "선릉이에게 받은 말린생선이 있으시면 저에게 전해 주세요.", { prev = false, next = false })
            return
        end

        if festival.is('중양절') then
            run_jungyang(me, npc)
            return
        end

        local t = datetime()
        local month = (t and t.month) and t.month or 1
        if month < 1 or month > 12 then
            month = 1
        end
        me:dialog(npc, "지금은 " .. tostring(month) .. "월이네요. 이 달 이야기는 준비중입니다.", { prev = false, next = false })
    end
}
