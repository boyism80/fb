-- @note Trash: 2_이벤트\토토.txt "부롱또또". #boorongtoto_*, $boorongtoto_*, callfunc BOORONGTOTO_* not implemented. Intro dialogs + list (자동/수동 참여, 당첨번호, 응모번호, 응모리스트, 당첨여부) with placeholder messages.

local JOIN_MONEY = 100

---@brief   NPC 부롱또또: lotto-style event. Intro dialogs + list; join/prize check logic omitted (server/account globals, callfunc).
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
function NPC_486(me, npc)
    local btn = me:dialog(npc, "현재 테스트 기간( ~ 2015. 6. 28)입니다. 당첨되어도 따로 상금/상품 지급은 되지 않습니다. 버그를 발견하면 제보 해 주세요.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "현재 [누적상금]전 의 상금이 \\n누적되어 있습니다. \\n\\n매주 발표되는 실제 로또번호를 기반으로 당첨자가 결정됩니다.\\n\\n월요일부터 금요일까지 참여가 가능합니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "(보너스번호는 당첨 제외)\\n\\n6개 숫자 : 용무기 8류 택1\\n5개 숫자 : 해골갑옷\\n4개 숫자 : 2500만 전\\n3개 숫자 : 500만 전", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, string.format("참여금액은 %d전 입니다.\\n현재 누적액 : (준비중) 전", JOIN_MONEY), {
        "자동으로 참여",
        "수동으로 참여",
        "=============================================",
        "[당첨번호] 회차 당첨번호 확인",
        "[응모번호] 회차 응모번호 확인",
        "[조회] 현재까지 응모한 유저들 확인하기",
        "=============================================",
        "당첨여부 확인",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 or sel == 1 then
        me:dialog(npc, "이미 참여 하셨네요. 한 계정에 한번만 참여가 가능합니다. (참여 기능은 준비중입니다.)", false, false)
    elseif sel == 3 or sel == 4 or sel == 5 or sel == 6 then
        me:dialog(npc, "준비중입니다.", false, false)
    elseif sel == 7 then
        me:dialog(npc, "당첨여부 확인은 준비중입니다.", false, false)
    else
        me:dialog(npc, "준비중입니다.", false, false)
    end
end
