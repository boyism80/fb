-- npc: 만리장성잡화상07
local enum = require('lib.enum')

function NPC_400(me, npc)
    ::NPC_400_000::
    local sel, btn = me:list(npc, "무슨일로 찾아왔는가?", {
        "금천지패와 금인패를 합쳐주십시오..",
        "금천인패와 금지패를 합쳐주십시오..",
        "금지인패와 금천패를 합쳐주십시오..",
    }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    local options = {
        { required = { ["녹비약(대)"] = 1, ["금천지패"] = 1, ["금인패"] = 1 }, need_msg = "금천지패와 금인패를 합치기 위해선 '녹비약(대)'가 필요하다네.. 자네는 가지고 있지 않군.." },
        { required = { ["녹비약(대)"] = 1, ["금천인패"] = 1, ["금지패"] = 1 }, need_msg = "금천인패와 금지패를 합치기 위해선 '녹비약(대)'가 필요하다네.. 자네는 가지고 있지 않군.." },
        { required = { ["녹비약(대)"] = 1, ["금지인패"] = 1, ["금천패"] = 1 }, need_msg = "금지인패와 금천패를 합치기 위해선 '녹비약(대)'가 필요하다네.. 자네는 가지고 있지 않군.." },
    }

    if sel < 1 or sel > 2 then
        goto NPC_400_000
    end

    local opt = options[sel]
    local required = opt.required

    local reward = nil
    if math.random(1, 100) <= 50 then
        reward = { ['item'] = { ["금천지인패"] = 1 } }
    end
    local code = me:exchange({ ['item'] = required }, reward)
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, opt.need_msg, false, false)
        goto NPC_400_000
    elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 금천지인패를 받을 수 없네. 자리 좀 비우고 다시 오게.", false, false)
        goto NPC_400_000
    elseif reward == nil then
        me:dialog(npc, "아니 실패해버렸잖아! 이런.. 미안하네..", false, false)
    else
        me:dialog(npc, "멋지게 성공했군. 하하하...", false, false)
    end
    goto NPC_400_000
end