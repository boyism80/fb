-- npc: 만리장성잡화상05
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        ::NPC_398_000::
        local sel, btn = me:list(npc, "무슨일로 찾아왔는가?", {
            "동천지패와 동인패를 합쳐주십시오..",
            "동천인패와 동지패를 합쳐주십시오..",
            "동지인패와 동천패를 합쳐주십시오..",
        }, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        local options = {
            { required = { ["녹비약(소)"] = 1, ["동천지패"] = 1, ["동인패"] = 1 }, need_msg = "동천지패와 동인패를 합치기 위해선 '녹비약(소)'가 필요하다네.. 자네는 가지고 있지 않군.." },
            { required = { ["녹비약(소)"] = 1, ["동천인패"] = 1, ["동지패"] = 1 }, need_msg = "동천인패와 동지패를 합치기 위해선 '녹비약(소)'가 필요하다네.. 자네는 가지고 있지 않군.." },
            { required = { ["녹비약(소)"] = 1, ["동지인패"] = 1, ["동천패"] = 1 }, need_msg = "동지인패와 동천패를 합치기 위해선 '녹비약(소)'가 필요하다네.. 자네는 가지고 있지 않군.." },
        }

        if sel < 1 or sel > 3 then
            goto NPC_398_000
        end

        local opt = options[sel]
        local required = opt.required

        local reward = nil
        if math.random(1, 100) <= 75 then
            reward = { ['item'] = { ["동천지인패"] = 1 } }
        end
        local code = me:exchange({ ['item'] = required }, reward)
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, opt.need_msg, { prev = false, next = false })
            goto NPC_398_000
        elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 동천지인패를 받을 수 없네. 자리 좀 비우고 다시 오게.", { prev = false, next = false })
            goto NPC_398_000
        elseif reward == nil then
            me:dialog(npc, "아니 실패해버렸잖아! 이런.. 미안하네..", { prev = false, next = false })
        else
            me:dialog(npc, "멋지게 성공했군. 하하하...", { prev = false, next = false })
        end
        goto NPC_398_000
    end
}
