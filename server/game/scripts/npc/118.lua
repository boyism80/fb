-- npc: 진진
local quest = require('lib.quest')
local server = require('lib.server')

function NPC_118(me, npc)
    local dq = me:quest(quest.QUEST_DETECTIVE)
    if dq and not dq:completed() then
        local ds = dq:step()
        if ds == 7 then
            ::NPC_118_0001::
            local btn = me:dialog(npc, "으잉? 그...그건 수..수사관 명찰 아닌가요? 도..도대체 무슨 일이죠?", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_118_0002::
            local sel, list_btn = me:list(npc, "나...나한테 무슨 보..볼 일이슈?", {
                "아니, 뭐 좀 물어볼게 있어서요.",
                "예? 전 지나가던 길인데요?",
            }, true)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if list_btn == DIALOG_RESULT.PREV then
                goto NPC_118_0001
            end
            if sel == 0 then
                dq:step(8)
                me:dialog(npc, "아..아니, 난 별로 할 말..두 없구! 아! 바빠! 바쁘다니까!", false, false)
            else
                me:dialog(npc, "빨리 지..지나가슈!!", false, false)
            end
            return
        end
        if ds == 9 then
            local sel, list_btn = me:list(npc, "무..무슨 일이요. 도대체? 나..난 죄없어!", {
                "최근 돈을 많이 버셨다죠?",
                "아, 그래요? 실례했습니다.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                return
            end
            sel, list_btn = me:list(npc, "주..주었다구, 돈자루를..! 줏..줏은 것 뿐이야!", {
                "습득물은 신고를 하셔야죠?",
                "아, 그래요? 실례했습니다.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                return
            end
            sel, list_btn = me:list(npc, "아..아냐, 줏..줏었다기 보다는 수고비로..", {
                "수고비요? 누가 왜 줬나요?",
                "아, 그래요? 실례했습니다.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                return
            end
            sel, list_btn = me:list(npc, "나..난 시키는대로 했을 뿐이야! 나..난!", {
                "시키는대로요? 누가 뭘 시켰죠?",
                "아, 그래요? 실례했습니다.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                return
            end
            sel, list_btn = me:list(npc, "으..아냐, 난 아무 말도 안했어! 안했다구!", {
                "이미 다 알고 왔습니다.",
                "아, 그래요? 실례했습니다.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                return
            end
            sel, list_btn = me:list(npc, "다..다 알고 왔다니? 뭐..뭘 알고 와?", {
                "부자 대감들과 나쁜 짓을 하셨죠?",
                "그렇게 잡아떼시다니, 나중에 두고 봅시다!",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                return
            end
            sel, list_btn = me:list(npc, "나..나쁜 짓이라니! 나..난 그런짓 안했어!", {
                "늦은 밤에 부자들에게 술대접을 받았죠?",
                "늦은 밤에 바닷가로 가는 걸 본 사람이 있어요.",
                "늦은 밤에 화산굴로 들어갔었죠?",
                "늦은 밤에 도둑질을 했었죠?",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 or sel == 2 or sel == 3 then
                me:dialog(npc, "어..얼른 가슈!", false, false)
                return
            end
            sel, list_btn = me:list(npc, "그..그건 바..바람 쐬려고...", {
                "거짓말 말아요!",
                "아, 그래요? 실례했습니다.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                return
            end
            sel, list_btn = me:list(npc, "저..정말인데..", {
                "부자들을 만나러 갔었죠?",
                "조개를 훔치러 갔었죠?",
                "섬에서 도망가려고 했죠?",
                "밀수선을 기다리고 있었죠?",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 or sel == 1 or sel == 2 then
                me:dialog(npc, "어..얼른 가슈!", false, false)
                return
            end
            sel, list_btn = me:list(npc, "아니야! 아니야! 난 모르고 한 일이라구!", {
                "정직하게 털어놓으세요.",
                "그러면, 어쩔 수 없군요.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 then
                me:dialog(npc, "나..난 정말 죄 없어!", false, false)
                return
            end
            ::NPC_118_0009::
            local btn = me:dialog(npc, "어..어쩔 수 없었어! 우리 어머니를 좀 더 잘 모시고 싶었을뿐이야!", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_118_0010::
            btn = me:dialog(npc, "그냥.. 그냥 시키는대로 궤짝을 날라다 배에 실어준 것 밖에 없다구..", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_118_0009
            end
            btn = me:dialog(npc, "(진진은 얼굴을 감싸쥐고 울기 시작했다.)", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            dq:step(10)
            me:dialog(npc, "(당신은 이번 사건에서 진진의 증언만을 기억하고, 진진의 죄는 잊기로 마음 먹었다.)", false, false)
            return
        end
        if ds > 0 and ds < 10 then
            me:dialog(npc, "무...무슨 일이오! 난 무언가를 훔치지 않았어!", false, true)
            return
        end
        if ds >= 10 and ds < 16 then
            me:dialog(npc, "(흑흑흑...)", false, true)
            return
        end
    end

    local q = me:quest(quest.QUEST_JINJIN)
    local lighthouse = me:quest(quest.QUEST_LIGHTHOUSE)

    if q == nil then
        if lighthouse == nil or lighthouse:completed() or lighthouse:step() ~= 1 then
            goto NPC_118_DEFAULT
        end
        if not me:has_items('초보도시락', 1) then
            goto NPC_118_DEFAULT
        end
        local sel, lb = me:list(npc, '잠깐만! 혹시 여유가 있다면 그 도시락을 나에게 주시지 않겠소?', { '네, 그러지요.', '주기 싫은데요.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        q = me:start_quest(quest.QUEST_JINJIN)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        local code = me:exchange(
            { ['item'] = { ['초보도시락'] = 1 } },
            { ['item'] = { ['선장의일기1'] = 1 } }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '도시락을 가지고 있지 않으시군요.', false, true)
            return
        elseif code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 선장의일기1을 줄 수 없네.', false, true)
            return
        end
        q:complete()
        lighthouse:step(2)
        me:push_achievement(32, '진백랑의 부탁을 들어주자. [2/6]', 7, 1)
        me:dialog(npc, '고맙소! 일이 급해서 도시락 만들 여유가 없었거든! 자, 이거라도 받으시오!', false, true)
        return
    end

    ::NPC_118_DEFAULT::
    me:dialog(npc, '일을 가야하는데 도시락을 깜빡 잊었네. 이를 어쩐담?! 이제와서 만들 시간도 없는데. 누가 초보 도시락이라도 팔지 않으려나.', false, true)
end
