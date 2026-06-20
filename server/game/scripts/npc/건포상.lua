local server = require('lib.server')

local ITEM_DEAD_CENTIPEDE = '죽은지네'
local ITEM_DRIED_CENTIPEDE = '말린지네'

function NPC_239(me, npc)
    ::NPC_239_0001::
    local btn = me:dialog(npc, '고기나 생선말고는 건포로 만들기 어렵지...\n\n지네 같은 것은 말리면 부서지고 없어져서, 하기가 어려운데...', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_239_0002::
    btn = me:dialog(npc, '자네가 가진 지네를 전부 주면 한번 시도해 봄세...', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_239_0001
    end

    local item = me:item(ITEM_DEAD_CENTIPEDE)
    local count = 0
    if item ~= nil then
        count = item:count()
    end
    if count < 7 then
        me:dialog(npc, '지네가 너무 적어서 만들수 없을 것 같네..', false, false)
        return
    end

    local rate
    if count >= 7 and count < 20 then
        rate = 30
    elseif count >= 20 and count < 50 then
        rate = 50
    elseif count >= 50 and count < 80 then
        rate = 70
    else
        rate = 90
    end
    if rate < 50 then
        rate = 50
    elseif rate > 100 then
        rate = 100
    end

    local r = math.random(1, 100)
    local reward = (r < rate) and { ['item'] = { [ITEM_DRIED_CENTIPEDE] = 1 } } or nil
    local code = me:exchange(
        { ['item'] = { [ITEM_DEAD_CENTIPEDE] = count } },
        reward
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '지네가 너무 적어서 만들수 없을 것 같네..', false, false)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 말린지네를 받을 수 없네.', false, false)
        return
    end
    if reward ~= nil then
        me:dialog(npc, '이거 쉽지 않구만... 간신히 만들었네... 자주 들리게나...', false, true)
    else
        me:dialog(npc, '이런... 지네가 모두 부숴저버렸군..', false, false)
    end
end