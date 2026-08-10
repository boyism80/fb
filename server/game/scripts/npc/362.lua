-- npc: 오선릉
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

local OSEONREUNG_REWARDS = { "티라노의알", "유리의장미", "유리의거울", "유리의팔찌", "유리의빗", "유리의안경" }
local ACHIEVEMENT_OSEONREUNG = 56

local function has_catch_jar(me)
    for i = 0, 10 do
        if me:has_items(string.format("채집통 [%d 마리]", i), 1) then
            return true
        end
    end
    return false
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
    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
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

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요? 저는 오선릉입니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('삼짇날') then
            run_samjit(me, npc)
        elseif festival.is('중화절') then
            run_junghwa(me, npc)
        else
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        end
    end
}
