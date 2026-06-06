-- 낙랑의두루마리7 사용 스크립트
local GOAL_LEVEL = 5

function ON_ACTIVATED_7(me, item)
    local quest = me:quest(QUEST_NAKRANG7)
    local btn
    local model = item:model()

    if quest == nil or quest:completed() then
        ::ON_ACTIVATED_7_0000::
        btn = me:dialog(model, '<임무>\n 레벨 5 만들기\n\n<내용>\n 잠시 퀴즈로 머리를 식혔으니, 다시 레벨업의\n세계로!! 레벨 5를 만들어보세요!!\n\n<보상>\n초심자의방패', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_7_0000
        end
        ::ON_ACTIVATED_7_0001::
        btn = me:dialog(model, '<힌트>\n 레벨 5를 만들면 직업을 선택하실 수 있습니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_7_0000
        end
        if quest == nil then
            quest = me:start_quest(QUEST_NAKRANG7)
            if quest == nil then
                return
            end
        end
        quest:step(1)
        return
    end

    local step = quest:step()
    if step ~= 1 then
        return
    end

    if me:level() >= GOAL_LEVEL then
        btn = me:dialog(model, '잘 하셨습니다!! 상으로\n초심자의방패를 드릴게요!!', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = { ['낙랑의두루마리7'] = 1 } },
            { ['item'] = { ['낙랑의두루마리8'] = 1, ['초심자의방패'] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(model, '소지품이 가득 찼습니다.', false, false)
            return
        end
        quest:complete()
        me:dialog(model, '<보상>\n\'초심자의방패\',\n\'낙랑의두루마리8\'을 얻다!!!', false, true)
    else
        me:dialog(model, '레벨 5를 만드는 것은 그다지\n힘들지 않습니다. 왕초보\n사냥터에서만 사냥해도 금방\n만드실 수 있을 꺼에요.', false, true)
    end
end