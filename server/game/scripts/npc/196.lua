-- npc: 고구려-섬선원
function NPC_196(me, npc)
    local sel, btn = me:list(npc, '무엇을 도와드릴까요?', {
        '출항시간에 대해서 알려주세요.',
        '배를 태워주세요.'
    })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        run_schedule_dialog(me, npc, '고구려에서 고균도로', '고균도')
        return
    end
    if sel ~= 1 then
        return
    end

    local t = datetime()
    local hour = t.hour
    local minute = t.minute
    if (hour % 2) ~= 1 then
        me:dialog(npc, '배가 선착장에 없습니다.', false, false)
        return
    end
    if minute < 40 or minute > 59 then
        me:dialog(npc, '배가 선착장에 없습니다.', false, false)
        return
    end

    ::NPC_196_0001::
    btn = me:dialog(npc, '배가 선착장에 정박해 있습니다.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_196_0002::
    local board_sel, list_btn = me:list(npc, '배에 승선하시길 원하십니까?', {
        '네, 승선시켜 주시기 바랍니다.',
        '아니오, 타지 않겠습니다.'
    }, true)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto NPC_196_0001
    end
    if board_sel == 1 then
        me:dialog(npc, '네, 안녕히 가십시오.', false, true)
        return
    end
    if board_sel ~= 0 then
        return
    end

    if me:level() < 21 then
        me:dialog(npc, '레벨 21이상만 이용 가능합니다.', false, true)
        return
    end
    if not me:has_items('고구려-고균도승선권', 1) then
        me:dialog(npc, '승선권이 없으면 선실에 탑승하실 수 없습니다.', false, true)
        return
    end
    if not me:rmitem('고구려-고균도승선권', 1, ITEM_DELETE_TYPE.GIVE) then
        return
    end

    btn = me:dialog(npc, '그럼, 선실에 승선시켜드리도록 하겠습니다.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc, '배가 출항한뒤 일정 시간 후에 고균도에 도착하게 됩니다. 즐거운 여행 되시길..', false, true)

    local map = name2map('고구려-고균도배선실')
    if map ~= nil then
        me:map(map, {math.random(2, 15), 7})
    end
end

function run_schedule_dialog(me, npc, route_text, dest_name)
    ::NPC_196_0010::
    local btn = me:dialog(npc, route_text .. ' 건너가는 배는 하루 총 12번입니다.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_196_0011::
    btn = me:dialog(npc, '2시부터 24시까지 매 2시간마다 ' .. dest_name .. '로 건너가는 배가 출항합니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_196_0010
    end
    ::NPC_196_0012::
    btn = me:dialog(npc, '출항하기전 미리 배에 오르고 있어야만 ' .. dest_name .. '로 갈 수 있음을 명심하시기 바랍니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_196_0011
    end
    ::NPC_196_0013::
    btn = me:dialog(npc, '감사합니다.', false, false)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_196_0012
    end
end
