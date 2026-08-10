-- npc: 백수인
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

local function run_junghwa(me, npc)
    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end
    if me:has_items("연", 1) then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    local button = me:dialog(npc, "중화절이 되니 몽연이형이 또 세시마을 대청소를 하는군요. 창고에서 또 뭐 꺼내달라고 할 텐데.. 귀찮은데..", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "연을 받아오라고 했다구요? 잠시만요.. 찾아볼께요....", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "............\n\n........여기있네.", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "혹시 또 뭐 더 찾아달라고 하면.. 직접와서 좀 찾으라고 해요.. 귀찮아요...", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("연", 1)
end

local function run_dongji(me, npc)
    local btn = me:dialog(npc, "일하기 싫어.. 놀고 싶어..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "요즘은 해가 짧아서 좋네요. 일을 덜해도 되거든요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "무슨 일이세요? 귀찮은데..", {
        "동지가 뭔가요?",
        "팥죽을 좀 끓여주세요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 1 then
        btn = me:dialog(npc, "양력으로 12월 12일이나 23일쯤 되면 밤의 길이가 가장 긴 날이 오는데, 이 날을 동지(冬至)라고 해요. 음력으론 11월이죠.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "동지에는 집에서 팥죽을 먹는 풍습이 있어요. 제가 하는 일은 음력 11월동안 팥죽을 끓여서 나눠주는거지요.", { prev = false, next = false })
        return
    end

    if sel == 2 then
        btn = me:dialog(npc, "팥죽을 끓이려면 팥이 있어야 되는데... 제가 팥을 준비를 못했네요. 귀찮아서..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "아마 칠석집의 주연이누나가 팥을 가지고 있을텐데.. 좀 받아와주시겠어요? 팥죽 끓이는건 제가 하죠..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if not me:has_items("팥", 1) then
            return
        end

        btn = me:dialog(npc, "엇. 팥을 받아오셨나보네요. 헤헤. 감사합니다. 그럼 팥죽을 끓여드릴께요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "끓이는데 시간이 좀 걸리니까 잠시만 기다리세요~", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "... ...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "... ... ...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "음.. 그러고보니 주연이누나에게 팥죽을 줘야 할꺼 같은데...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "팥은 충분하니 팥죽 두 그릇은 되겠네요. 한 그릇은 드시고, 한 그릇은 주연이누나에게 좀 전해주세요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "사실 이번에 팥죽 드시는 분들에게 제가 고약을 드려야 하는데.. 주연이 누나한테서 아직 안받아 왔거든요.. 귀찮아서...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "주연이누나에게 팥죽을 드리고, 고약도 받으세요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local code = me:exchange(
            { ['item'] = { ["팥"] = 1 } },
            { ['item'] = { ["팥죽"] = 2 } }
        )
        if code == enum.exchange_result.LACK_COST then
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 팥죽을 드릴 수 없습니다.", { prev = false, next = false })
            return
        end
        me:message("팥죽 받았습니다.", MESSAGE_TYPE.NOTIFY)
    end
end

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요? 저는 백수인입니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('동지') then
            run_dongji(me, npc)
        elseif festival.is('중화절') then
            run_junghwa(me, npc)
        else
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        end
    end
}
