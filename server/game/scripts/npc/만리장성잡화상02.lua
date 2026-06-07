function NPC_395(me, npc)
    local required = { ["녹비약(소)"] = 1, ["동천패"] = 1, ["동지패"] = 1, ["동인패"] = 1 }

    ::NPC_395_000::
    local sel, btn = me:list(npc, "무슨일로 찾아왔는가?", {
        "동천패, 동지패, 동인패를 합쳐주십시오..",
        "도움이 될 만한 정보 없습니까?",
    }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 0 then
        local reward = nil
        if math.random(1, 100) <= 75 then
            reward = { ['item'] = { ["동천지인패"] = 1 } }
        end
        local code = me:exchange({ ['item'] = required }, reward)
        if code == EXCHANGE_RESULT.LACK_COST then
            if not me:has_items({ ["동천패"] = 1, ["동지패"] = 1, ["동인패"] = 1 }) then
                me:dialog(npc, "여보게, 자네는 동천패, 동지패, 동인패를 모두 가져오지 않았잖나.", false, false)
            else
                me:dialog(npc, "동천지인패를 만들기 위해서는 녹비약(소)가 필요하다네. 자네는 가지고 있지 않은 것 같은데?", false, false)
            end
            goto NPC_395_000
        elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 동천지인패를 받을 수 없네. 자리 좀 비우고 다시 오게.", false, false)
            goto NPC_395_000
        elseif reward == nil then
            me:dialog(npc, "아니 실패해버렸잖아! 이런.. 미안하네..", false, false)
        else
            me:dialog(npc, "멋지게 성공했군. 하하하...", false, false)
        end
    elseif sel == 1 then
        me:dialog(npc, "고구려엔 왈숙네라는 주모가 있다면서? 어떤 주모이길래 중국에 까지 소문이 나는걸까?", false, false)
    end
    goto NPC_395_000
end
