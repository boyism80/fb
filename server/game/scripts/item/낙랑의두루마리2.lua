-- 낙랑의두루마리2 사용 스크립트
function ON_ACTIVATED_2(me, item)
    local quest = me:quest(QUEST_NAKRANG2)
    local btn
    local model = item:model()

    if quest == nil or quest:completed() then
        ::ON_ACTIVATED_2_0000::
        btn = me:dialog(model, '<임무>\n 도톨 2개 사용\n\n<내용>\n 첫번째 임무는 도톨을 사용하는 것!! 도톨\n2개를 드렸으니, 두루마리를 사용하듯이 이\n도톨도 사용해보세요!! 시간은 5초 드립니다!!\n\n<보상>\n 목도, 경험치 50 (레벨5미만)', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::ON_ACTIVATED_2_0001::
        btn = me:dialog(model, '<힌트>\n 도톨을 사용하기 위해서는 오른쪽의\n\'소지품(단축키i)\'을 클릭하신 다음 제가 드린\n도톨을 더블클릭하면 됩니다. 단축키 \'u(use)\'를\n누른 다음, 도톨이 있는 슬롯의 알파벳을\n누르셔도 됩니다. \'b\'슬롯에 있다면, \'u\'를\n누르신 후 \'b\'를 누르시면 됩니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_2_0000
        end
        if quest == nil then
            quest = me:start_quest(QUEST_NAKRANG2)
            if quest == nil then
                return
            end
        end
        quest:step(1)
        quest:progress(0)
        return
    end

    local step = quest:step()
    if step ~= 1 then
        return
    end

    local progress = quest:progress()
    if progress < 2 then
        me:dialog(model, '실패하셨군요!! 좀 더 연습한 후 다시\n시도해보세요..', true, false)
        return
    end

    me:dialog(model, '잘 하셨습니다!! 상으로\n목도를 드릴게요!!', false, true)

    if me:mkitem('낙랑의두루마리3', 1) == nil or me:mkitem('초심자의목도', 1) == nil then
        me:dialog(model, '소지품이 가득 찼습니다.', false, false)
        return
    end
    me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)

    if me:level() < 5 then
        me:exp(me:exp() + 50)
        me:dialog(model, '<보상>\n\'목도\',\n\'낙랑의두루마리3\'을 얻다!!!\n경험치 50 상승!!', false, true)
    else
        btn = me:dialog(model, '<보상>\n\'목도\',\n\'낙랑의두루마리3\'을 얻다!!!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
    end

    quest:progress(0)
    quest:complete()
end