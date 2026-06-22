-- item: 낙랑의두루마리5
-- 낙랑의두루마리5 사용 스크립트
local quest = require('lib.quest')
local server = require('lib.server')

local GOAL = 5

local function parse_param(param)
    if param == nil or param == '' then
        return 0, 0
    end
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return 0, 0
    end
    return math.min(tonumber(a) or 0, GOAL), math.min(tonumber(b) or 0, GOAL)
end

local function set_param(q, squirrel, rabbit)
    q:param(string.format('%d,%d', math.min(squirrel, GOAL), math.min(rabbit, GOAL)))
end

function ON_ACTIVATED_5(me, item)
    local q = me:quest(quest.QUEST_NAKRANG5)
    local btn
    local model = item:model()

    if q == nil or q:completed() then
        ::ON_ACTIVATED_5_0000::
        btn = me:dialog(model, '<임무>\n 토끼와 다람쥐 사냥\n\n<내용>\n 이제, 사냥터에 왔으니 사냥을 해 봐야지요!!\n주변에, \'토끼\'와 \'다람쥐\'가 보이실 것입니다.\n일단 몬스터 앞에 서서 스페이스바를\n연타하시면 마구 공격합니다. \'토끼\' 5마리와\n\'다람쥐\' 5마리씩 사냥해보세요~\n\n<보상>\n 초심자의귀마개, 경험치 200 (레벨5미만)', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_5_0000
        end
        ::ON_ACTIVATED_5_0001::
        btn = me:dialog(model, '<힌트>\n 마우스로 몬스터를 마구 클릭해도 공격합니다.\n떨어진 아이템은 마우스로 클릭하거나, 아이템\n위에 서서 ,키를 누르시면 주울 수 있습니다 ', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_5_0000
        end
        btn = me:dialog(model, '팁\n도토리나 토끼고기는\n푸줏간에 파실 수 있습니다.\n직업을 가질 때, 도토리가\n10개가 필요하니 10개는\n남겨두세요..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_5_0001
        end
        if q == nil then
            q = me:start_quest(quest.QUEST_NAKRANG5)
            if q == nil then
                return
            end
        end
        q:step(1)
        set_param(q, 0, 0)
        return
    end

    local step = q:step()
    if step ~= 1 then
        return
    end

    local squirrel, rabbit = parse_param(q:param())

    if squirrel >= GOAL and rabbit >= GOAL then
        ::ON_ACTIVATED_5_0002::
        btn = me:dialog(model, '잘 하셨습니다!! 상으로\n초심자의머리띠를 드릴게요!!', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local give_exp = (me:level() < 5)
        local code = me:exchange(
            { ['item'] = { ['낙랑의두루마리5'] = 1 } },
            { ['item'] = { ['낙랑의두루마리6'] = 1, ['초심자의머리띠'] = 1 }, ['exp'] = give_exp and 200 or 0 }
        )
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(model, '소지품이 가득 찼습니다.', false, false)
            return
        end
        if give_exp then
            btn = me:dialog(model, '<보상>\n\'초심자의귀마개\'\n\'낙랑의두루마리6\'을 얻다!!!\n경험치 200 상승!!', false, true)
        else
            btn = me:dialog(model, '<보상>\n\'초심자의귀마개\'\n\'낙랑의두루마리6\'을 얻다!!!', false, true)
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_5_0002
        end
        set_param(q, 0, 0)
        q:complete()
        return
    end

    ::ON_ACTIVATED_5_0003::
    btn = me:dialog(model, string.format('사냥한 토끼 : %d / %d\n사냥한 다람쥐 : %d / %d', rabbit, GOAL, squirrel, GOAL), false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::ON_ACTIVATED_5_0004::
    btn = me:dialog(model, '몬스터 앞으로 가서\n스페이스바를 연타하거나,\n마우스로 몬스터를 클릭하면\n공격할 수 있습니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_5_0003
    end
    btn = me:dialog(model, '귀엽지만.. \n눈물을 머금고\n조금만 더 사냥해보세요..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_5_0004
    end
end
