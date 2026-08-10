-- npc: 백주연
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

local function run_junghwa(me, npc)
    local q = quest.get_annual(me, quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end
    if me:has_items("실패", 1) then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    local button = me:dialog(npc, "벌써 몽연오빠가 청소하는 때가 되었나보네요.. 실패를 가져오라고 했다구요? 잠시만요.....", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "................여기있네요.", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "그럼 행복한 봄날 되세요!", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("실패", 1)
end

local function run_dongji(me, npc)
    local menu1 = me:has_items("팥죽", 1) and "팥죽을 드리러 왔어요." or "팥을 받으러 왔어요."
    local sel, list_btn = me:list(npc, "안녕하세요. 어떻게 오셨나요?", {
        menu1,
        "그냥 인사차...",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 2 then
        me:dialog(npc, "안녕하세요~! 잘 지내고 계신가요?", { prev = false, next = false })
        return
    end

    if sel ~= 1 then
        return
    end

    local has_key = me:has_items("동지창고열쇠", 1)
    local has_cinnamon = me:has_items("계피", 1)
    local has_porridge = me:has_items("팥죽", 1)

    -- Handle rewards/turn-ins before key grant.
    if has_porridge then
        local btn = me:dialog(npc, "와아 맛있어보이는 팥죽이네요. 감사합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "그럼 계피로 만든 고약을 드릴께요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "매달 얻을 수 있는 다른 아이템들과 같이 보관하시면 좋은 일이 있을꺼에요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = { ["팥죽"] = 1 } },
            { ['item'] = { ["고약"] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 고약을 드릴 수 없습니다.", { prev = false, next = false })
            return
        end
        me:message("고약 받았습니다.", MESSAGE_TYPE.NOTIFY)
        return
    end

    if has_cinnamon then
        local btn = me:dialog(npc, "아아.. 그런 일이 있었군요. 후추귀신이 계피귀신을 괴롭히고 계피를 빼앗아가서 계피가 없었던 건가요...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "음.. 그럼 팥을 드릴께요.. 수인이에게 팥죽을 끓여달라고 하세요. 계피는 감사합니다. ^^", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = { ["계피"] = 1 } },
            { ['item'] = { ["팥"] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 팥을 드릴 수 없습니다.", { prev = false, next = false })
            return
        end
        me:message("팥 받았습니다.", MESSAGE_TYPE.NOTIFY)
        return
    end

    if has_key then
        me:dialog(npc, "동지집에 가시면 창고가 있는데, 동지창고열쇠로 창고 안에서 계피를 좀 찾아와 주시겠어요? 그 동안 저는 팥을 찾아 볼께요.", { prev = false, next = false })
        return
    end

    local btn = me:dialog(npc, "네? 수인이가 팥을 받아오라고 했다구요? 이런 게으른 녀석! 직접 받으러 오진 않고 손님에게 심부름을 시키다니...", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "수인이와 저는 나이는 같은데 제가 생일이 빨라서 누나소리를 듣는데요. 저녀석은 너무 게을러서 큰일이네요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "아, 팥이 필요하다고 하셨죠? 음.. 그런데 사실은 저도 부탁이 있는데요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "제가 지금 고약을 만들고 있는데.. 고약을 만드는 데 쓰이는 계피가 모자라요. 이상하게 창고에 계피가 모자라더군요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "동지집에 가시면 창고가 있는데, 제가 열쇠를 드릴테니 안에서 계피를 좀 찾아와 주시겠어요? 그 동안 저는 팥을 찾아 볼께요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if me:mkitem("동지창고열쇠", 1) == nil then
        me:dialog(npc, "소지품이 가득 차서 동지창고열쇠를 드릴 수 없습니다.", { prev = false, next = false })
        return
    end
    me:message("동지창고열쇠 받았습니다.", MESSAGE_TYPE.NOTIFY)
end

local function run_chilseok(me, npc)
    local sel, list_btn = me:list(npc, "무슨일로 찾아오셨어요?", {
        "칠석에 대해 알려주세요.",
        "요리를 해주세요!",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 1 then
        local btn = me:dialog(npc, "칠석(七夕)은 은하수의 양 끝 둑에 살고 있는 견우와 직녀가 1년에 한 번 만나는 날이에요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "너무나 사랑하던 두 사람은 옥황상제의 노여움으로 1년에 한번 칠석 전날 밤에 은하수를 건너 만나게 되죠.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "이 때 까치와 까마귀가 날개를 펴서 다리를 놓아주는데, 이 다리를 오작교라고 해요. 그래서 칠석날 아침에 비가 내리면 견우직녀 상봉의 눈물이구요, 저녁에 비가 내리면 이별의 눈물이래요..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "그런데, 사람들이 사냥을 너무 많이 해서 까치와 까마귀의 숫자가 부족해서 올해 견우와 직녀가 못 만날지도 모른다는 얘기가 있던데...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "음.. 칠성당에 고사를 지내러 가보시면 뭔가 알게 될지도 모르지요..", { prev = false, next = false })
        return
    end

    if sel == 2 then
        local btn = me:dialog(npc, "어머, 요리를 해달라구요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local cook_sel, cook_btn = me:list(npc, "무엇을 요리해 드릴까요?", {
            "호박부침을 해주세요..",
            "잉어를 잡아왔어요. 구워주세요!",
        }, { prev = false })
        if cook_btn == DIALOG_RESULT.QUIT or cook_sel == nil then
            return
        end

        if cook_sel == 1 then
            if not me:has_items("식용호박", 1) then
                me:dialog(npc, "식용호박을 가져오시면 맛있는 호박부침을 해드릴께요~~~", { prev = false, next = false })
                return
            end
            btn = me:dialog(npc, "음.. 그럼 식용호박을 가져오셨겠죠?\n\n그럼 호박부침을 해드릴께요.. 잠시만 기다려주세요~~", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local code = me:exchange(
                { ['item'] = { ["식용호박"] = 1 } },
                { ['item'] = { ["호박부침"] = 1 } }
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, "식용호박을 가져오시면 맛있는 호박부침을 해드릴께요~~~", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 호박부침을 드릴 수 없습니다.", { prev = false, next = false })
                return
            end
            me:dialog(npc, "랄랄라~~~~\n\n맛있는 호박부침이 완성되었습니다~~~ 맛있게 드세요~~", { prev = false, next = false })
            return
        end

        if cook_sel == 2 then
            if not me:has_items("잉어", 1) then
                me:dialog(npc, "잉어를 가져오시면 맛있는 잉어구이를 해드릴께요~~~", { prev = false, next = false })
                return
            end
            btn = me:dialog(npc, "잉어를 잡아오셨어요?", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "그럼 잉어를 구워드릴께요~~~", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "흠흠..", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local code = me:exchange(
                { ['item'] = { ["잉어"] = 1 } },
                { ['item'] = { ["잉어구이"] = 1 } }
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, "잉어를 가져오시면 맛있는 잉어구이를 해드릴께요~~~", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 잉어구이를 드릴 수 없습니다.", { prev = false, next = false })
                return
            end
            me:dialog(npc, "잉어가 맛있게 구워졌네요!", { prev = false, next = false })
        end
    end
end

local function run_kimjang(me, npc)
    -- yeondailyy screenshots: list prompt + 칠석이벤트 → salt dialog
    local sel, list_btn = me:list(npc, "안녕하세요. 어떻게 오셨나요?", {
        "칠석이벤트",
        "견우와 직녀의 만남 축하",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel ~= 1 then
        return
    end

    if me:has_items("소금", 1) then
        return
    end

    local btn = me:dialog(npc, "김장김치를 담그나보네요.. 소금은 여기 있습니다. 맛있는 김치 담그세요~", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    me:mkitem("소금", 1)
end

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요? 저는 백주연입니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('김장') then
            run_kimjang(me, npc)
        elseif festival.is('동지') then
            run_dongji(me, npc)
        elseif festival.is('칠석') then
            run_chilseok(me, npc)
        elseif festival.is('중화절') then
            run_junghwa(me, npc)
        else
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        end
    end
}
