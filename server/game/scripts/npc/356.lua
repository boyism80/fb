-- npc: 정어언
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')

local function run_seollal(me, npc)
    local sel, list_btn = me:list(npc, " 새해계획은 세우셨나요?", {
        "설날엔 뭘 하나요?",
        "떡국을 먹고싶어요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        local btn = me:dialog(npc, " 설날엔 아침 일찍 일어나서 세수를 하고 '설빔'을 입죠. 설빔은 지난해 가을부터 옷감을 준비했다가 설날까지 옷을 장만하여 입는거랍니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, " 설빔을 입고, 차례를 지낸후에 어른들께 새해 인사를 드리는데, 이것을 세배라고 하구요, 세배를 마치고 나면 아침 식사로 떡국을 먹지요. 아, 조상님 묘지에 가서 벌초도 해야겠죠?", { prev = false, next = false })
        return
    end

    if sel == 2 then
        local btn = me:dialog(npc, " 벌초를 하고 오시면 제가 떡국을 끓여드릴께요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, " 잡초를 베다보면 '잡초풀'이 나오는데, 이 풀을 제게 가져다 주세요. 떡국을 끓일 때 불쏘시개로 유용하게  쓰이거든요. 100개 정도면 적당할 것 같아요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if not me:has_items("잡초풀", 100) then
            me:dialog(npc, " 잡초를 베어오시고 잡초풀을 가져와 주세요.. 100개 정도면 적당할 것 같아요. ", { prev = false, next = false })
            return
        end

        local code = me:exchange(
            { ['item'] = { ["잡초풀"] = 100 } },
            { ['item'] = { ["떡국"] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, " 잡초를 베어오시고 잡초풀을 가져와 주세요.. 100개 정도면 적당할 것 같아요. ", { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 떡국을 드릴 수 없습니다.", { prev = false, next = false })
            return
        end

        btn = me:dialog(npc, " 이 떡국 먹고 올해도 좋은 일만 가득하시길 바랄께요..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, " 휴우.. 우리 남편에게도 떡국을 드려야 할텐데... 저희 남편은 칠성당에서 고사나 차례를 준비하는 분이랍니다..", { prev = false, next = false })
    end
end

local function run_junghwa(me, npc)
    local q = quest.get_annual(me, quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end
    if me:has_items("널", 1) then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    local button = me:dialog(npc, "몽연이가 청소를 한다구요? 그럼 저는 널뛰기 판자를 부탁드릴께요..", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "이 널은 세시마을에 있는 여자애들이 명절때마다 쓰던 널이랍니다. 이젠 너무 낡아서 새로 장만해야겠네요.", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "그럼 몽연이에게 잘 전해주세요!", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("널", 1)
end

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요? 저는 정어언입니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('설날') then
            run_seollal(me, npc)
        elseif festival.is('중화절') then
            run_junghwa(me, npc)
        else
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        end
    end
}
