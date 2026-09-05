-- npc: 백남인
local enum = require('lib.enum')
local festival = require('lib.festival')
local yudu_gosa = require('lib.yudu_gosa')
local magpie_bridge = require('lib.magpie_bridge')

local ACHIEVEMENT_SEOLBIM = 566

local function run_seollal(me, npc)
    local sel, list_btn = me:list(npc, " 어쩐일로 찾아오셨습니까?", {
        "정월 대보름이 뭐에요?",
        "떡국을 드릴께요, 드세요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 1 then
        local btn = me:dialog(npc, " 정월 대보름은 1월 15일 입니다. 말 그대로 1월의 보름이라는 뜻입니다. 1월을 정월이라고 하지요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, " 정월 대보름에는 해가 뜨기전에 친구나 친척을 만나 이름을 부르고, 상대방이 대답하면 '내 더위 사세요~'라고 하면 그 해 여름동안 더위를 먹지 않는다고 하죠.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, " 대보름엔 찰밥을 먹고, 아이들은 바람개비를 가지고 노는 날 이기도 하고, 밤에 달이 뜨면 달을 보며 소원을 비는 것도 빼놓을 수 없는 풍습입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, " 세시마을에서 달맞이를 할 수 있는 곳은 세시마을 남쪽 무덤을 지나 달맞이 다리를 건너가시면 달맞이 고개가 있지요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, " 달맞이 하실땐 꼭 설빔을 입고 가세요..", { prev = false, next = false })
        return
    end
    if sel == 2 then
        local armor = (me:gender() == GENDER.MALE) and "남자설빔" or "여자설빔"
        local code = me:exchange(
            { ['item'] = { ["떡국"] = 1 } },
            { ['item'] = { [armor] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, " 이번해는 정말 바빠서 떡국 한 그릇도 먹지 못하고 있네요. 이거 참...", { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "설빔을 지급할 수 없습니다.", { prev = false, next = false })
            return
        end

        me:push_achievement(ACHIEVEMENT_SEOLBIM, '설빔을 받았다.', 7, 16)

        local btn = me:dialog(npc, " 허허.. 이런 감사합니다. 바빠서 올해 떡국을 어떻게 먹나 했더니.. 이렇게 전해주시다니 정말 감사합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, " 어디보자.. 이거 저도 뭐 보답을 해드려야 할텐데..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, " 설빔이 없으신가 보네요? 그럼 제가 준비한 설빔을 드리면 되겠네요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, " 설빔을 잘 간직하셔서 다른 명절에 받으신 아이템들과 같이 잘 모아두세요..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, " 설빔 잘 입으시고 한해동안 복 많이 받으세요 ~", { prev = false, next = false })
    end
end

local function give_chilseok_reward(me, npc)
    local reward = (math.random(1, 2) == 1) and "직녀의바늘" or "견우의피리"
    local btn = me:dialog(npc, me:name() .. "님이 주신 음식으로 올해도 무사히 제를 올렸군요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if reward == "직녀의바늘" then
        btn = me:dialog(npc, "자.. 그럼 직녀가 사용하는 바늘을 드리도록 하겠습니다.", { prev = false, next = true })
    else
        btn = me:dialog(npc, "자.. 그럼 견우가 만든 피리를 드리도록 하겠습니다.", { prev = false, next = true })
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "매달 얻을 수 있는 아이템을 모두 모으면 좋은일이 생길꺼에요. 칠석에는 견우의피리나 직녀의바늘중 하나만 가지고 계시면 돼요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if me:mkitem(reward, 1) == nil then
        me:dialog(npc, "소지품이 가득 차서 드릴 수 없습니다.", { prev = false, next = false })
        return
    end
    local map = name2map("세시마을")
    if map == nil then
        map = id2map(10932)
    end
    if map ~= nil then
        me:map(map, math.random(52, 59), math.random(46, 50))
    end
end

local function run_chilseok(me, npc)
    local t = datetime()
    local hour = (t and t.hour) or 0
    local minute = (t and t.minute) or 0
    local second = (t and t.second) or 0

    -- Ritual reward window: hours 3/9/15/21, minute >= 10
    local ritual_hour = (hour == 3 or hour == 9 or hour == 15 or hour == 21)
    -- Bridge settle window (first 5 minutes of 0/6/12/18)
    local bridge_hour = (hour == 0 or hour == 6 or hour == 12 or hour == 18)

    if ritual_hour then
        if minute >= 10 then
            give_chilseok_reward(me, npc)
        else
            me:dialog(npc, "아직 제를 올리지 않았습니다. 잠시 기다려 주세요....", { prev = false, next = false })
        end
        return
    end

    if bridge_hour then
        local rem = magpie_bridge.bridge_build_remaining(t)
        if rem == 0 then
            me:dialog(npc, "저런, 때를 놓치셨군요...", { prev = false, next = false })
            return
        end

        local btn = me:dialog(npc, "칠석에는 견우와 직녀가 만나는데.. 올해는 까마귀 숫자가 부족해서 오작교가 만들어질 수 없을것 같다는데...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = me:list(npc, "오작교 만드는데 참여해주시겠어요?", {
            "네, 도와드리죠.",
            "아니요.. 바빠서..",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel ~= 1 then
            me:dialog(npc, "아직 제를 올리지 않았습니다. 잠시 기다려 주세요....", { prev = false, next = false })
            return
        end

        local where_name = (me:gender() == GENDER.MALE) and "견우" or "직녀"
        local house = where_name .. "의집"
        btn = me:dialog(npc, "지금 올라가시면 " .. where_name .. "의집으로 들어가게 됩니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, where_name .. "의집을 나오셔서 구름위로 걸어가시면 다리를 만드실 수 있는데...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local minutes_left = (rem ~= nil) and math.max(1, math.ceil(rem / 60)) or 5
        btn = me:dialog(npc, "지금부터 " .. tostring(minutes_left) .. "분안에 참여하지 못하면 다리만드는데 참여하실수가 없어요. 주의하시길...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "혹시 문제가 생기면 노란비서를 사용하세요. 드리겠습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if me:mkitem("노란비서", 3) == nil then
            me:dialog(npc, "소지품이 가득 차서 노란비서를 드릴 수 없습니다.", { prev = false, next = false })
            return
        end
        local map = name2map(house)
        if map ~= nil then
            -- Message/timer before map (thread hop).
            if rem ~= nil and rem > 0 then
                me:timer(rem, TIMER_TYPE.DECREASE)
            end
            me:map(map, math.random(7, 12), math.random(6, 12))
        end
        return
    end

    me:dialog(npc, "지금은 제를 올리는 시간이 아닙니다...", { prev = false, next = false })
end

return {
    on_click = function(me, npc)
        if festival.is('설날') then
            run_seollal(me, npc)
        elseif festival.is('칠석') then
            run_chilseok(me, npc)
        elseif festival.is('유두') then
            yudu_gosa.try_reward(me, npc)
        else
            me:dialog(npc, "지금은 칠성당 관련 이벤트 기간이 아닙니다.", { prev = false, next = false })
        end
    end
}
