local quest = require('lib.quest')
function NPC_379(me, npc)
    local q = me:quest(quest.QUEST_DETECTIVE)

    if q == nil then
        ::NPC_379_0001::
        local btn = me:dialog(npc, "아이구, 정말이지. 요즘들어 일거리만 끝도없이 끝도없이 쏟아진단 말이야.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_379_0002::
        btn = me:dialog(npc, "정말로 외부인은 재앙이야. 재앙덩어리라구.", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_379_0001
        end
        local sel, list_btn = me:list(npc, "이젠 정말 지쳐서 쓰러질지도 모르겠어. 에휴.", {
            "언제나 불평뿐이군요. 힘을 좀 내봐요.",
            "이번에는 또 무슨 일인가요?",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            me:dialog(npc, "에휴... ?", false, false)
            return
        end
        if sel == 1 then
            me:dialog(npc, "자네가 좀 도와줬으면 좋겠는데...", false, false)
            return
        end
        btn = me:dialog(npc, "요즘들어 문화재 유출이 너무 많아. 분명 어딘가 외부로 우리 문화재를 팔아넘기는 녀석이 있는거야.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_379_0005::
        btn = me:dialog(npc, "그것도 조직적이라는 냄새가 풀풀 난다구. 한 사람이 그렇게 많은 양을 모으고 또 판다는건 불가능해.", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_379_0005
        end
        ::NPC_379_0006::
        btn = me:dialog(npc, "그런 파렴치범은 꼭 잡아서 처벌해야할텐데 말이야. 문제는 도대체 누가 범인인지 실마리조차 찾을 수 없다는거지.", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_379_0005
        end
        sel, list_btn = me:list(npc, "아. 도대체 어떻게 해야할런지 모르겠다. 모르겠다구.", {
            "말만 하지 말고 행동을 해야죠!",
            "그거야 갑자기 부자가 된 사람을 의심해봐야죠.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            me:dialog(npc, "자네가 좀 도와줬으면 좋겠는데...", false, false)
            return
        end
        ::NPC_379_0008::
        btn = me:dialog(npc, "음? 과연 듣고보니 그럴듯하군! 문화재는 비싸게 거래되니까 말이야! 쉽게 부자가 될 수 있겠지!", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_379_0007::
        btn = me:dialog(npc, "이런 중요한 실마리를 단숨에 떠올려내다니. 자네는 분명 명석한 두뇌를 가지고 있는게 틀림없어!", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_379_0008
        end
        btn = me:dialog(npc, "보아하니 행동력과 뛰어난 두뇌를 지닌 것 같은데, 임시 수사관으로써 한 번 범인을 밝혀내보지 않겠나?", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        sel, list_btn = me:list(npc, "임시 수사관이 되어주지 않겠나?", {
            "바빠요. 자기 일은 스스로 하세요.",
            "기꺼이 도와드리죠.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            me:dialog(npc, "그렇고말고... 내 일은 내가 해야겠지.", false, false)
            return
        end
        q = me:start_quest(quest.QUEST_DETECTIVE)
        if q == nil then
            me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
            return
        end
        if q then
            q:step(1)
        end
        me:push_achievement(54, "문화재 유출범을 찾아라.", 7, 16)
        btn = me:dialog(npc, "좋았어! 그럼 이제부터 자네는 임시 수사관이야. 이 명찰을 달고 있으면 섬사람들이 수사에 협조해줄걸세.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "그리고 나와 동생들이라고 놀고 있지는 않을테니까 가끔 들러서 상황도 알려주시게. 우리도 얻은 정보를 그때그때 알려줄테니까.", false, false)
        return
    end

    if q:completed() then
        me:dialog(npc, me:name() .. " 아닌가? 자네덕에 모든게 해결되어서 좋다고. 하하하!", false, false)
        return
    end

    local step = q:step()

    if step == 0 then
        local btn = me:dialog(npc, "아이구, 정말이지. 요즘들어 일거리만 끝도없이 끝도없이 쏟아진단 말이야.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "정말로 외부인은 재앙이야. 재앙덩어리라구.", false, false)
        return
    end

    if step < 1 or step > 15 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    local sel, list_btn = me:list(npc, "머리를 맞대고 정보 교환을 시작한다.", {
        "새로운 정보를 확인한다.",
        "경과를 보고한다.",
        "지금은 할 이야기가 없다.",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 2 then
        return
    end

    if sel == 0 then
        if step == 1 then
            ::NPC_379_0010::
            local btn = me:dialog(npc, "일단 여태까지 자네가 추리해온 것처럼 가장 수상한건 부자 대감들이네.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "아무리 부자라고 해도 요즘들어 재산이 몇 배씩 불어나고 있었거든. 이유를 몰라서 어디서 금이라도 캐오나 싶었더니만.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "자네 말대로 이제는 그 뚱보 부자들이 제일 의심스럽네. 자네의 그 추리력을 발휘해서 진상을 밝혀주게나!", false, false)
        elseif step == 4 then
            ::NPC_379_0020::
            local btn = me:dialog(npc, "뭐? 문화재를 선물로? 중국과 교역이라고? 무슨 소리야? 자네가 말한 그 문화재는 이 섬에 얼마 남지 않은", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_379_0021::
            btn = me:dialog(npc, "엄청 귀중한 문화재야! 그걸 외부인에게 선물로 줬다니! 제 정신이 아니로구만, 그래!", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_379_0020
            end
            ::NPC_379_0022::
            btn = me:dialog(npc, "거기다가 외부와의 교역은 우리에게 허가를 맡아야 하는데, 부자들은 한 마디 말도 없었다구! 이런 뜬금없는 소리는", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_379_0021
            end
            me:dialog(npc, "난생처음 들어보네! 세상에! 아..아이구! 혈..혈압이 머리 꼭대끼까지 솟아오르네...아이구 아이구;", false, false)
            if q then
                q:step(5)
            end
        else
            me:dialog(npc, "준비중입니다.", false, false)
        end
        return
    end

    if sel == 1 then
        if step == 10 then
            ::NPC_379_0030::
            local btn = me:dialog(npc, "밀수선은 다만 물건만 가지러 오지는 않을게야. 그래서야 수지타산이 맞지 않을테니까.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "그래, 자네 말대로 무언가 팔 물건도 가져왔겠지... 근데 우리 섬에서 외부 물건을 탐낼만한 사람이", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "부자들말고 따로 있을까? 하긴, 부자들만 외부 세상 물건에 관심 있으란 법은 없긴하지.", false, false)
            if q then
                q:step(11)
            end
        elseif step == 15 then
            ::NPC_379_0040::
            local btn = me:dialog(npc, "대단하군! 자네의 명석한 두뇌를 믿길 잘했지! 설마 이렇게 간단히 사건을 처리할 줄은 몰랐네!", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_379_0041::
            btn = me:dialog(npc, "중국의 밀수선이 없어지면, 제 아무리 부자들이라고 해도 어쩔 수가 없겠지! 하하하하하하하!", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_379_0040
            end
            ::NPC_379_0042::
            btn = me:dialog(npc, "이제 부자들에게 벌금만 징수하면 끝나겠군. 좋아! 자네에게 명수사관 칭호와 함께 우리 섬주민들을 대표하여", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_379_0041
            end
            me:dialog(npc, "감사의 뜻으로 10만전을 드리지! 정말 수고했네! 큰일을 해냈어! 하하하하하하하!", false, true)
            if q then
                q:step(16)
                q:complete()
            end
            me:money(me:money() + 100000)
            me:push_achievement(54, "명수사관 칭호를 획득!", 7, 16)
        else
            me:dialog(npc, "준비중입니다.", false, false)
        end
    end
end
