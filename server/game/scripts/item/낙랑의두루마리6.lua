-- 낙랑의두루마리6 사용 스크립트
local function quiz_wrong_and_exit(me, model, wrong_msg)
    me:dialog(model, wrong_msg, false, true)
    me:dialog(model, '바람의나라에 대한 공부를\n조금 더 한 뒤 다시\n도전하세요!!!', false, true)
end

function ON_ACTIVATED_6(me, item)
    local model = item:model()
    local btn

    if me:level() < 3 then
        me:dialog(model, '레벨제한\n 레벨 3 이상인 유저만\n펼처보실 수 있습니다.', false, true)
        return
    end

    ::ON_ACTIVATED_6_0000::
    btn = me:dialog(model, '<임무>\n 잠깐 퀴즈~!!\n\n<내용>\n 레벨을 올리시느라 힘드셨죠? 그럼 잠시\n쉬어가는 의미에서.. 퀴즈를 풀어보는 것이\n어떨까요?\n\n<보상>\n 초심자의반지, 경험치 400 (레벨5미만)', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_6_0000
    end
    ::ON_ACTIVATED_6_0001::
    btn = me:dialog(model, '<힌트>\n 퀴즈에 대한 정답을 선택하기만 하면 됩니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_6_0000
    end

    local sel1, list_btn = me:list(model, '그럼, 첫번째 문제입니다.!!\n\n<문제>\n 게임 내 게시판의 단축키는?', { '1) i', '2) o', '3) b' }, false)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel1 ~= 2 then
        quiz_wrong_and_exit(me, model, '틀렸습니다!! \'i\'는\n소지품창의 단축키, \'o\'는\n문을 열 때의 단축키 입니다.')
        return
    end
    me:dialog(model, '잘 하셨습니다!! \'i\'는\n소지품창의 단축키, \'o\'는\n문을 열 때의 단축키, \'b\'가\n게시판을 볼 때의 단축키이죠..', false, true)

    local sel2, list_btn2 = me:list(model, '그럼, 두번째 문제!!\n\n<문제>\n 다음 중 바람의나라 운영자의 아이디는?', { '1) 주몽', '2) 주몽지엠', '3) 주몽GM' }, false)
    if list_btn2 == DIALOG_RESULT.QUIT then
        return
    end
    if sel2 ~= 2 then
        quiz_wrong_and_exit(me, model, '틀렸습니다!! 바람의나라\n운영자는 아이디 뒤에 영문자\n\'GM\'이 붙습니다.')
        return
    end
    me:dialog(model, '맞추셨습니다!! 바람의나라\n운영자는 아이디 뒤에 영문자\n\'GM\'이 붙습니다.', false, true)

    local sel3, list_btn3 = me:list(model, '세번째 문제~!\n\n<문제>\n 바람의나라 운영자가 비밀번호를\n물었을 경우 해야 할 행동을?', { '1) 친절하게 알려준다', '2) 반만 알려준다', '3) 알려주지 않는다' }, false)
    if list_btn3 == DIALOG_RESULT.QUIT then
        return
    end
    if sel3 ~= 2 then
        quiz_wrong_and_exit(me, model, '틀렸습니다!! 바람의나라\n운영자는 절대로 비밀번호를\n묻지 않습니다. 따라서\n절대로 알려주면 안됩니다.')
        return
    end
    me:dialog(model, '맞추셨습니다!! 바람의나라\n운영자는 절대로 비밀번호를\n묻지 않습니다. 따라서\n절대로!! 알려주면 안됩니다.', false, true)

    local sel4, list_btn4 = me:list(model, '드디어 마지막 문제!!\n\n<문제>\n 바람의나라에서 유용한 도움말을\n볼 수 있는 단축키는?', { '1) F1', '2) F2', '3) F10' }, false)
    if list_btn4 == DIALOG_RESULT.QUIT then
        return
    end
    if sel4 ~= 0 then
        quiz_wrong_and_exit(me, model, '틀렸습니다!! F2는\n순위리스트, F10은 설정의\n단축키입니다.')
        return
    end

    me:dialog(model, '맞추셨습니다!! F1은 도움말,\nF2는 순위리스트, F10은\n설정의 단축키입니다.', false, true)
    me:dialog(model, '잘 하셨습니다!! 상으로\n초심자의반지를 드릴게요!!', false, true)

    me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    me:mkitem({ ['낙랑의두루마리7'] = 1, ['초심자의반지'] = 1 })
    if me:level() < 5 then
        me:dialog(model, '<보상>\n\'초심자의반지\',\n\'낙랑의두루마리7\'을 얻다!!!\n경험치400상승', false, true)
        me:exp(me:exp() + 400)
    else
        me:dialog(model, '<보상>\n\'초심자의반지\',\n\'낙랑의두루마리7\'을 얻다!!!', false, true)
    end
end