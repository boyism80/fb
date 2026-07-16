-- npc: 만리장성잡화상06
function NPC_399(me, npc)
::NPC_399_000::
    local sel, btn = me:list(npc, "무슨 용건인가?", {"금패, 은패, 동패는 무엇인가요?", "뭐 재미있는 이야기라도 해주십시오."}, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        local button = me:dialog(npc, "금, 은, 동은 모두 금속이 아닌가? 화려한 금은 부귀, 영화를 상징하고, 단단한 은은 영원을 상징하고, 이곳저곳에 쓸모가 많은 동은 문명을 상징한다네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "동으로 된 패는 같은 것이 다섯개 모여서 은패로 합쳐지고, 은으로 된 패는 다시 다섯개가 모여 금패로 합쳐진다고 하네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "그렇다면 여러문명이 모여 영원한 문명이 되고, 여러개의 영원이 모이면 부귀, 영화가 되는 것일까? 뭔가 앞뒤가 맞지 않는 말인거 같지만, 전설에는 분명히 그렇게 되어있지.", false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
    elseif sel == 2 then
        local messages = {
            "흉노족 두령은 생긴건 무섭게 생겼지만 알고보면 말이 잘 통하는 똑똑한 사람이라던데.. 역시 사람은 겉만 보곤 알 수 없는건가?",
            "자네, 부여성 왕궁엔 가보았는가? 요즘은 부여성의 왕궁에 일반인도 출입이 가능하다고 하던데..",
            "대방성 대장장이 녀석이 검정 깃발을 혼자 가지고 있다는 소문이 있어.",
            "선비족 녀석들.. 고구려와 중국을 왕래하는 상인들을 습격한다고 하더라고. 나쁜놈들..",
            "아아.. 나도 용궁에 한번 가보고 싶은데... 어디로 가야하지?",
        }
        local msg = messages[math.random(1, #messages)]
        local button = me:dialog(npc, msg, false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
    goto NPC_399_000
end