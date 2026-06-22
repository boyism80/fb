-- npc: 간수장
local server = require('lib.server')

function NPC_308(me, npc)
    local button = me:dialog(npc, "엣헴...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, "이십오만전을 가져왔군!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local rewards = { "곰방대", "작약주", "신작약주" }
    local given
    for _, item_name in ipairs(rewards) do
        if math.random(1, 10) <= 2 then
            given = item_name
            break
        end
    end
    local reward = given and { ['item'] = { [given] = 1 } } or nil
    local code = me:exchange(
        { ['money'] = 250000 },
        reward
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, me:name() .. "놈아 장사 하루이틀 해 보냐, 일단 돈 이십오만전을 주면 생각해 보겠다.", false, false)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 받을 수 없네.", false, false)
        return
    end
    if reward ~= nil then
        me:dialog(npc, "잘 받았다. 이 일도 수입이 꽤 좋단 말이야", false, false)
    else
        me:dialog(npc, "음... 이것 가지고는 조금 부족한데... 돈이 더 있어야 하겠다.\\n안됐구나.", false, false)
    end
end
