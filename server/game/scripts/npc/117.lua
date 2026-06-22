-- npc: 조염
local quest = require('lib.quest')
local server = require('lib.server')

local ITEM_BONG_BOOK = '봉래산전설'

function NPC_117(me, npc)
    local dq = me:quest(quest.QUEST_DETECTIVE)
    if dq and not dq:completed() then
        local ds = dq:step()
        if ds == 3 or ds == 4 then
            local sel, list_btn = me:list(npc, "아니, 그건 수사관 명찰이 아닌가!\n대단하군!", {
                "참. 봉래산 전설말이에요.",
                "어라? 들고계신 조각상은 뭐예요?",
                "이 섬의 부자들에 대해 아시는게 있나요?",
                "요즘 수상쩍은 일은 없었나요?",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 then
                goto NPC_117_BOOK
            end
            if sel == 1 then
                local btn = me:dialog(npc, "아, 이거? 이 섬의 부자들이 우리 중국과의 교역을 시작한 기념으로 내게 선물해줬다네.", false, true)
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "굉장히 귀한 물건이라고 하던데... 참 마음에 든다네.", true, false)
                if btn == DIALOG_RESULT.PREV then
                    return
                end
                dq:step(4)
            elseif sel == 2 then
                local btn = me:dialog(npc, "부자들이란... 변혁을 두려워하지 않는 사람들이더군. 외국 문물을 받아들이는데 망설임이 없었어.", false, true)
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "역시 부자는 아무나 되는게 아니지. 그야말로 거상의 풍모랄까. 그런 모습에 반해서 당장 중국의 상인들을 소개시켜 줬다네.", true, false)
                if btn == DIALOG_RESULT.PREV then
                    return
                end
                dq:step(4)
            else
                local btn = me:dialog(npc, "수상쩍은 일? 글쎄, 나는 이 섬에 대해서 연구하는 중이라, 거의 모든 일에 집중하고 있는 탓에", false, true)
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "무엇이 수상쩍은지, 무엇이 정상인지 분간이 힘들 정도라네. 무릇 사실이란 객관적으로 기록해야하는 법이니까.", true, false)
                if btn == DIALOG_RESULT.PREV then
                    return
                end
                dq:step(4)
            end
            return
        end
        if ds == 6 then
            local sel, list_btn = me:list(npc, "그래, 하시는 일은 잘 되어가나?", {
                "부자들은 교역 허가를 받은 적이 없던데요?",
                "네, 그럭저럭이요.",
                "아, 봉래산 전설 때문에요.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 then
                dq:step(7)
                me:dialog(npc, "응? 그럴리가! 부자 대감들이 이렇게 무역 허가증까지 제출했는걸?", false, false)
            end
            return
        end
        if ds == 14 then
            local sel, list_btn = me:list(npc, "응? 무슨일인가? 뭐? 밀수? 하하하. 농담도 잘하는구만", {
                "확실한 증거가 있습니다.",
                "하긴 그렇죠?",
                "아. 봉래산 전설 말이에요.",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 then
                ::NPC_117_0001::
                local btn = me:dialog(npc, "뭣이?! 감히 나를 속이고 밀수선을 부르다니! 이런 발칙한 사람들을 봤나!", false, true)
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                ::NPC_117_0002::
                btn = me:dialog(npc, "알겠네! 황제 폐하께 서신을 보내서 당장 은익이란 밀수선을 찾아 처벌해야겠네!", true, true)
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if btn == DIALOG_RESULT.PREV then
                    goto NPC_117_0001
                end
                dq:step(15)
                me:dialog(npc, "정말 수고가 많았네! 대국의 위신을 바로 잡을 수 있게 도와주다니! 명수사관다워! 정말 고맙네!", false, false)
            end
            return
        end
    end

    ::NPC_117_BOOK::
    local q = me:quest(quest.QUEST_BONG_BOOK)
    local has_book = me:has_items(ITEM_BONG_BOOK, 1)

    if q == nil then
        local selected = me:list(npc, '내가 책 한권 드릴테니 한번 읽어나 보시게나.', { '예, 주세요.', '아니오, 책이라면 질색이라서..' })
        if selected == nil then
            return
        end
        if selected == 0 then
            local q = me:start_quest(quest.QUEST_BONG_BOOK)
            if q == nil then
                me:dialog(npc, '퀘스트 시작 실패', false, true)
                return
            end
            if me:mkitem(ITEM_BONG_BOOK, 1) == nil then
                me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', false, true)
                return
            end
            me:dialog(npc, '여기있네. 그럼 재밌게 읽게나.', false, true)
        else
            me:dialog(npc, '그러면 쓰나.. 책을 읽어야 마음에 양식이 쌓이는 것이네.', false, true)
        end
        return
    end

    if not has_book then
        me:dialog(npc, '뭐야? 책을 잃어버려? 이런 고약한.. 내 저번엔 다시 줬지만, 이번엔 그냥은 못주겠네.', true, true)
        local selected = me:list(npc, '1000전을 내면 다시 한권 주지.', { '봉래산전설을 다시 읽고싶어요. 1000전에 주세요.', '돈이 없어서..' })
        if selected == nil or selected ~= 0 then
            return
        end
        local code = me:exchange(
            { ['money'] = 1000 },
            { ['item'] = { [ITEM_BONG_BOOK] = 1 } }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '돈이 부족하지 않나?', false, true)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', false, true)
            return
        end
        me:dialog(npc, '다시 잘 읽어봐!', false, true)
    else
        me:dialog(npc, '어때? 봉래산전설은 재미있나?', false, true)
    end
end
