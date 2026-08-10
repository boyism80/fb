-- npc: 오선릉
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

local OSEONREUNG_REWARDS = { "티라노의알", "유리의장미", "유리의거울", "유리의팔찌", "유리의빗", "유리의안경" }
local ACHIEVEMENT_OSEONREUNG = 56
local ACHIEVEMENT_SEOTDAL = 562

local OUTGOING = { '곶감', '김장김치', '말린생선' }

local function has_catch_jar(me)
    for i = 0, 10 do
        if me:has_items(string.format("채집통 [%d 마리]", i), 1) then
            return true
        end
    end
    return false
end

local function seotdal_flags(q)
    local p = (q ~= nil and q:param()) or ''
    local y, w, d, b = string.match(p, '^(%d+):(%d+):(%d+):(%d+)$')
    if y ~= nil then
        return tonumber(w) or 0, tonumber(d) or 0, tonumber(b) or 0
    end
    -- Legacy "w:d:b" without year
    w, d, b = string.match(p, '^(%d+):(%d+):(%d+)$')
    return tonumber(w) or 0, tonumber(d) or 0, tonumber(b) or 0
end

local function set_seotdal_flags(q, w, d, b)
    local year = festival.lunar_year()
    q:param(string.format('%d:%d:%d:%d', year, w, d, b))
end

local function grant_missing_outgoing(me)
    local to_give = {}
    for _, name in ipairs(OUTGOING) do
        if not me:has_items(name, 1) then
            to_give[name] = 1
        end
    end
    if next(to_give) == nil then
        return true, false
    end
    if me:mkitem(to_give) == nil then
        return false, true
    end
    return true, true
end

local function run_samjit(me, npc)
    local sel, list_btn = me:list(npc, "안녕하세요? 좋은 봄날이죠?", {
        "나비채를 좀 주세요!",
        "화면을 드릴께요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        if has_catch_jar(me) then
            me:dialog(npc, "이미 채집통을 가지고 계신 것 같아요. 명주와 명지에게 나비를 많이 잡아다 주세요.", { prev = false, next = false })
            return
        end

        local btn = me:dialog(npc, "나비채와 채집통이라.. 찾아볼께요.. \n\n어디보자... (뒤적뒤적)", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "여기 있네요. 이 채집통엔 나비가 10마리 까지 들어가니까.. 많이 잡아서 명주하고 명지에게 잘 전해주세요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "주의하실건, 채집통을 흔들면 나비가 빠져나갈수도 있으니 조심하셔야 해요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if me:mkitem({ ["나비채"] = 1, ["채집통 [0 마리]"] = 1 }) == nil then
            me:dialog(npc, "소지품이 가득 차서 드릴 수 없습니다.", { prev = false, next = false })
        end
        return
    end

    if sel == 2 then
        if not me:has_items("맛있는화면", 1) then
            me:dialog(npc, "화창한 날씨죠?", { prev = false, next = false })
            return
        end

        local code = me:exchange(
            { ['item'] = { ["맛있는화면"] = 1 } },
            { ['item'] = { ["버들피리"] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, "화창한 날씨죠?", { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 버들피리를 드릴 수 없습니다.", { prev = false, next = false })
            return
        end

        local btn = me:dialog(npc, "아.. 명주와 명지가 화면을 줬다구요.. 이런 고마워서.. 헤헤.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "감사합니다! 저도 감사의 뜻으로.. 제가 만든 버들피리를 드릴께요. 가지고 계시면 좋은 일이 있을겁니다!", { prev = false, next = false })
    end
end

local function run_junghwa(me, npc)
    local q = quest.get_annual(me, quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    if q:progress() >= 1 then
        if me:has_items("제기", 1) then
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
            return
        end
        local button = me:dialog(npc, "전 대청소가 정말 싫어요. 발명할때 사용할 재료들은 잡동사니가 아니란 말이에요! 저에겐 꼭 필요한건데도 몽연이형은 다 치우라고 하고..", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "음.. 그런데 몽연이형이 다른 집들에서 이것저것 받아달라고 했죠? 그거.. 저에게 주시면 제가 좋은 아이템 드릴 수 있는데...", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "몽연이형이 달라고 한 제기는 줘야겠죠....", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "칫. 몽연이형은 발명의 소중함을 모른단 말이야...", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        me:mkitem("제기", 1)
        return
    end

    if me:has_items("연", 1) and me:has_items("깨진김장독", 1) and me:has_items("찌그러진냄비", 1) and me:has_items("널", 1) and me:has_items("실패", 1) then
        local reward_name = OSEONREUNG_REWARDS[math.random(1, 6)]
        local button = me:dialog(npc, "연, 실패, 깨진김장독, 찌그러진냄비, 널을 가지고 계시군요! 그걸 저에게 주시면.. " .. name_with(reward_name, '을', '를') .. " 드릴 수 있는데...", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        local cost = { ['item'] = { ["연"] = 1, ["실패"] = 1, ["깨진김장독"] = 1, ["찌그러진냄비"] = 1, ["널"] = 1 } }
        local reward = { ['item'] = { [reward_name] = 1 } }
        local code = me:exchange(cost, reward)
        if code == enum.exchange_result.LACK_COST then
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 " .. name_with(reward_name, '을', '를') .. " 받을 수 없어요. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
            return
        end
        q:progress(1)
        me:push_achievement(ACHIEVEMENT_OSEONREUNG, "오선릉에게 잡동사니를 전해주고 아이템을 받았다.", 7, 4)
        return
    end

    if me:has_items("제기", 1) then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    local button = me:dialog(npc, "전 대청소가 정말 싫어요. 발명할때 사용할 재료들은 잡동사니가 아니란 말이에요! 저에겐 꼭 필요한건데도 몽연이형은 다 치우라고 하고..", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "음.. 그런데 몽연이형이 다른 집들에서 이것저것 받아달라고 했죠? 그거.. 저에게 주시면 제가 좋은 아이템 드릴 수 있는데...", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "몽연이형이 달라고 한 제기는 줘야겠죠....", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "칫. 몽연이형은 발명의 소중함을 모른단 말이야...", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("제기", 1)
end

local function run_seotdal(me, npc)
    local q = quest.get_annual(me, quest.QUEST_SEOTDAL_GIFTS)
    if q ~= nil and q:completed() then
        me:dialog(npc, "선물을 잘 전해 주셔서 고마워요. 복조리는 잘 간직해 두세요.", { prev = false, next = false })
        return
    end

    if q ~= nil and q:step() >= 1 then
        local w, d, b = seotdal_flags(q)
        local options = { "선물을 다시 주세요." }
        if w == 1 and d == 1 and b == 1 then
            table.insert(options, 1, "선물을 다 전해줬어요.")
        end

        local sel, list_btn = me:list(npc, "도와드릴 일이 있나요?", options, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end

        local choice = options[sel]
        if choice == "선물을 다 전해줬어요." then
            if not me:has_items('귤', 1) or not me:has_items('제기', 1) then
                me:dialog(npc, "귤과 제기를 받아 오셔야 해요. 촌장부인과 오단미에게 선물을 전해주세요.", { prev = false, next = false })
                return
            end

            local code = me:exchange(
                { ['item'] = { ['귤'] = 1, ['제기'] = 1 } },
                { ['item'] = { ['복조리'] = 1 } }
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, "귤과 제기를 받아 오셔야 해요.", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 복조리를 드릴 수 없습니다.", { prev = false, next = false })
                return
            end

            q:complete()
            festival.mark_completed_year(q)
            me:push_achievement(ACHIEVEMENT_SEOTDAL, "섣달에 모두에게 선물을 전달하자", 7, 1)
            me:dialog(npc, "고마워요! 이것은 복조리예요. 세시투구를 만들 때 쓰게 될 거예요. 잘 간직해 두세요.", { prev = false, next = true })
            return
        end

        -- Re-grant missing outgoing gifts (edible / lost).
        local ok, given = grant_missing_outgoing(me)
        if not ok then
            me:dialog(npc, "소지품이 가득 차서 선물을 드릴 수 없습니다.", { prev = false, next = false })
            return
        end
        if not given then
            me:dialog(npc, "이미 선물을 가지고 계신 것 같아요. 김장김치는 촌장부인에게, 곶감은 오단미에게, 말린생선은 백남신에게 전해주세요.", { prev = false, next = false })
            return
        end
        me:dialog(npc, "여기 있어요. 김장김치는 촌장부인에게, 곶감은 오단미에게, 말린생선은 백남신에게 전해주세요.", { prev = false, next = true })
        return
    end

    local btn = me:dialog(npc, "12월은 섣달 또는 서웃달이라고 하고, 12월 마지막 날은 섣달그믐이라고 하지요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "섣달그믐은 다른 말로 제석 혹은 제야라고 해요. 모든 것이 제거되는 밤, 청산하는 저녁이라는 뜻이죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "섣달그믐에는 세찬이라고 하여 집집마다 음식이나 물건을 준비하여 서로 주고 받는 풍습이 있다고 해요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "도와줄 일을 물어보자", {
        "네.. 도와드리죠",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel ~= 1 then
        return
    end

    q = me:start_quest(quest.QUEST_SEOTDAL_GIFTS)
    if q == nil then
        me:dialog(npc, "퀘스트를 시작할 수 없습니다.", { prev = false, next = false })
        return
    end
    q:step(1)
    set_seotdal_flags(q, 0, 0, 0)

    if me:mkitem({ ['곶감'] = 1, ['김장김치'] = 1, ['말린생선'] = 1 }) == nil then
        me:dialog(npc, "소지품이 가득 차서 선물을 드릴 수 없습니다.", { prev = false, next = false })
        return
    end

    me:dialog(npc, "김장김치는 촌장부인에게, 곶감은 오단미에게, 말린생선은 백남신에게 전해주세요.", { prev = false, next = true })
end

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요? 저는 오선릉입니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('섣달') then
            run_seotdal(me, npc)
        elseif festival.is('삼짇날') then
            run_samjit(me, npc)
        elseif festival.is('중화절') then
            run_junghwa(me, npc)
        else
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        end
    end
}
