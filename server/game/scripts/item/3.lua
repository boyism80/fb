-- item: 낙랑의두루마리3

local quest = require('lib.quest')

local enum = require('lib.enum')

local WEAPON_NAME = '초심자의목도'

return {
    on_activated = function(me, item)
        local q = me:quest(quest.QUEST_NAKRANG3)
        local btn
        local model = item:model()

        if q == nil or q:completed() then
            ::ON_ACTIVATED_3_0000::
            if me:gender() == GENDER.MALE then
                btn = me:dialog(model, '<임무>\n 목도착용\n\n<내용> \n도톨을 사용하는 방법은 잘 익히셨지요? 이번엔\n목도를 착용해 봅시다! 도톨과 같은 방법으로\n사용하시면 착용이 된답니다!! 자.. 시간은 5초\n드립니다!!\n\n<보상>\n 초심자의남자갑주, 경험치 50 (레벨5미만)', { prev = false, next = true })
            else
                btn = me:dialog(model, '<임무>\n 목도착용\n\n<내용> \n도톨을 사용하는 방법은 잘 익히셨지요? 이번엔\n목도를 착용해 봅시다! 도톨과 같은 방법으로\n사용하시면 착용이 된답니다!! 자.. 시간은 5초\n드립니다!!\n\n<보상>\n 초심자의여자갑주, 경험치 50 (레벨5미만)', { prev = false, next = true })
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto ON_ACTIVATED_3_0000
            end
            ::ON_ACTIVATED_3_0001::
            btn = me:dialog(model, '<힌트>\n 목도를 착용하기 위해서는 오른쪽의\n\'소지품(단축키i)\'을 클릭하신 다음 제가 드린\n목도를 더블클릭하면 됩니다. 단축키 \'u(use)\'를\n누른 다음, 목도가 있는 슬롯의 알파벳을\n누르셔도 됩니다. \'c\'슬롯에 있다면, \'u\'를\n누르신 후 \'c\'를 누르시면 됩니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto ON_ACTIVATED_3_0000
            end
            if q == nil then
                q = me:start_quest(quest.QUEST_NAKRANG3)
                if q == nil then
                    return
                end
            end
            q:step(1)
            return
        end

        local step = q:step()
        if step ~= 1 then
            return
        end

        local weapon = me:weapon()
        if weapon == nil or weapon:model():name() ~= WEAPON_NAME then
            me:dialog(model, '실패하셨군요!! 좀 더 연습한 후 다시\n시도해보세요..', { prev = true, next = false })
            return
        end

        if me:gender() == GENDER.MALE then
            me:dialog(model, '잘 하셨습니다!! 상으로\n초심자의남자갑주를 드릴게요!!', { prev = false, next = true })
        else
            me:dialog(model, '잘 하셨습니다!! 상으로\n초심자의여자갑주를 드릴게요!!', { prev = false, next = true })
        end

        local armor_name = (me:gender() == GENDER.MALE) and '초심자의남자갑주' or '초심자의여자갑주'
        local code = me:exchange(
            { ['item'] = { ['낙랑의두루마리3'] = 1 } },
            { ['item'] = { ['낙랑의두루마리4'] = 1, [armor_name] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(model, '소지품이 가득 찼습니다.', { prev = false, next = false })
            return
        end

        if me:level() < 5 then
            me:exp(me:exp() + 50)
        end

        if me:gender() == GENDER.MALE then
            if me:level() < 5 then
                me:dialog(model, '<보상>\n\'초심자의남자갑주\',\n\'낙랑의두루마리4\'을 얻다!!!\n경험치 50 상승!!', { prev = false, next = true })
            else
                ::ON_ACTIVATED_3_0002::
                btn = me:dialog(model, '<보상>\n\'초심자의남자갑주\',\n\'낙랑의두루마리4\'을 얻다!!!', { prev = true, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if btn == DIALOG_RESULT.PREV then
                    goto ON_ACTIVATED_3_0002
                end
            end
        else
            if me:level() < 5 then
                me:dialog(model, '<보상>\n\'초심자의여자갑주\',\n\'낙랑의두루마리4\'을 얻다!!!\n경험치 50 상승!!', { prev = false, next = true })
            else
                ::ON_ACTIVATED_3_0003::
                btn = me:dialog(model, '<보상>\n\'초심자의여자갑주\',\n\'낙랑의두루마리4\'을 얻다!!!', { prev = true, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if btn == DIALOG_RESULT.PREV then
                    goto ON_ACTIVATED_3_0003
                end
            end
        end

        q:complete()
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
