local server = require('lib.server')
local OPTIONS = {
    { item = "동동주", cost = 200 },
    { item = "팔십세주", cost = 4000 },
    { item = "백세주", cost = 10000 },
}

function NPC_373(me, npc)
    local sel, list_btn = me:list(npc, "수련이 힘들텐데 술 한잔 하는게 어떻소? 공짜로 줄수는 없으니 소림사에 시주를 좀 해주었으면 하오. 어떤 술을 원하시오?", {
        "동동주 (200전)",
        "팔십세주 (4000전)",
        "백세주 (10000전)",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 2 then
        return
    end

    local opt = OPTIONS[sel + 1]
    local code = me:exchange(
        { ['money'] = opt.cost },
        { ['item'] = { [opt.item] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "안타깝지만 자네 수중에 충분한 돈이 없구만. 돈을 가지고 다시 찾아오시오.", false, true)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 줄 수 없구만.", false, false)
        return
    end

    me:dialog(npc, "여기있소. 피땀흘려 수련을 한다면 언젠가는 꼭 보상을 받을것이오.", false, true)
    me:dialog(npc, "수련장은 소림사 뒤쪽으로 가보면 승려들이 수련하고 있을것이오.", false, true)
end
