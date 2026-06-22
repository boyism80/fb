-- npc: 만리장성잡화상08
local server = require('lib.server')

function NPC_401(me, npc)
    local required = { ["녹비약(중)"] = 1, ["은천패"] = 1, ["은지패"] = 1, ["은인패"] = 1 }

    ::NPC_401_000::
    local sel, btn = me:list(npc, "무슨일로 찾아왔는가?", {
        "은천패, 은지패, 은인패를 합쳐주십시오..",
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
        if math.random(1, 100) <= 60 then
            reward = { ['item'] = { ["은천지인패"] = 1 } }
        end
        local code = me:exchange({ ['item'] = required }, reward)
        if code == server.EXCHANGE_RESULT.LACK_COST then
            if not me:has_items({ ["은천패"] = 1, ["은지패"] = 1, ["은인패"] = 1 }) then
                me:dialog(npc, "여보게, 자네는 은천패, 은지패, 은인패를 모두 가져오지 않았잖나.", false, false)
            else
                me:dialog(npc, "은천지인패를 만들기 위해서는 녹비약(중)가 필요하다네. 자네는 가지고 있지 않은 것 같은데?", false, false)
            end
            goto NPC_401_000
        elseif code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 은천지인패를 받을 수 없네. 자리 좀 비우고 다시 오게.", false, false)
            goto NPC_401_000
        elseif reward == nil then
            me:dialog(npc, "아니 실패해버렸잖아! 이런.. 미안하네..", false, false)
        else
            me:dialog(npc, "멋지게 성공했군. 하하하...", false, false)
        end
    elseif sel == 1 then
        me:dialog(npc, "난 은으로된 천패, 지패, 인패는 합칠 수 있지만, 천지패와 인패를 합친다던가, 천인패와 지패를 합치는 건 못한다네.", false, false)
    end
    goto NPC_401_000
end
