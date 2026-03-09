
function NPC_460(me, npc)
    if gv("check") == 25 and me:has_items("훈제연어", 5) then
        local btn = me:dialog(npc, "난 깊은계곡을 지키고 있는 곰이다웅... 그런데 너는 보니까 임무를 수행중인것처럼 보이는데웅...?", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = me:list(npc, "앗, 임무 수행중이라구웅? 훈제연어 5개만 주면 되는데... 내가 가진 아이템들과 바꿀생각이 있냐웅?", {
            "당근케이크",
            "(임무 완료 후 사용 가능...)",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            local code = me:exchange(
                { ['item'] = { ["훈제연어"] = 5 } },
                { ['item'] = { ["당근케이크"] = 10 } }
            )
            if code == EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, "훈제연어를 잃어버린것같구웅! 분명 너는 임무에서 받은 훈제연어를 나한테 가져와야 했을텐데웅?", false, false)
                return
            elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 당근케이크를 줄 수 없겠다웅!", false, false)
                return
            end
            me:dialog(npc, "자, 당근케이크를 받았으니... 다시 임무를 수행하러 가봐도 된다웅!!", false, false)
        end
        return
    end

    local btn = me:dialog(npc, "난 깊은계곡을 지키고 있는 곰이다웅...", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "연어가 먹고싶은데... 맛있는연어를 먹으면 혼날것 같고... 혹시 가지고 있는 훈제연어라도 있냐웅?", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "내가 가진 아이템들과 바꿀생각이 있냐웅? 훈제연어랑 바꿔줄 생각이 있는데웅...", {
        "우루삼",
        "당근케이크",
        "비서같은것들",
        "알사탕",
        "(추가 예정...)",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 4 then
        me:dialog(npc, "세상의 다양한 물건을 모으고 있으니... 다음에 새로 모은게 있으면 알려주겠다웅.", false, false)
        return
    end

    me:dialog(npc, "해당 교환은 준비중입니다. 홈페이지에 문의해 주세요.", false, false)
end
