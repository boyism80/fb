local quest = require('lib.quest')
local server = require('lib.server')

local function pyosin_story(me, npc)
    local ACHIEVEMENT_PYOSIN = 30
    local story_items = {
        { doc = '표신궁고문서1', diary = '대장군의일기1', money = 10000 },
        { doc = '표신궁고문서2', diary = '대장군의일기2', money = 20000 },
        { doc = '표신궁고문서3', diary = '대장군의일기3', money = 30000 },
        { doc = '표신궁고문서4', diary = '대장군의일기4', money = 40000 },
        { doc = '표신궁고문서5', diary = '대장군의일기5', money = 50000 },
        { doc = '표신궁고문서6', diary = '대장군의일기6', money = 60000 },
    }
    local q = me:quest(quest.QUEST_PYOSIN)
    local btn, sel
    
    if q == nil then
        ::NPC_250_0002::
        btn = me:dialog(npc, '헉! 누...누...... 누구시오!', false, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0003::
        sel = me:list(npc, '...사람이였구려. 이거 실례했소이다. 여긴 인적이 드문 곳이라 아무도 오질 않지요. 혹시 귀신은 아닌가 해서 그만 놀라고 말았소이다. 허허.', { '귀신? 귀신이 나옵니까?', '놀래켜서 죄송합니다. 전 이만...' })
        if sel == nil or sel == 0 then 
            return
        end
        if sel == 2 then
            me:dialog(npc, '아니네. 허허. 그럼 잘가보게.', false, false)
            return
        end
        ::NPC_250_0004::
        btn = me:dialog(npc, '모르셨소? 내 간단히 설명해드리지. 저 멀리 표신궁이 보이지요? 옛날 수많은 사람들을 학살한 대장군의 성이지. 그 대장군이 얼마나 포악했는가 하면, 대장군이 죽은 지금도 사람들이 이 성 근처에 가까지 오지 않으려 하는 것만 봐도 알 수 있지 않소?', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0005::
        btn = me:dialog(npc, '이런 곳이라면 어디에 언제 귀신이 나타나도 이상하지 않지... 난 연구를 해야 하니 이 곳에 있을 수 밖에 없지만.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0004 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0006::
        btn = me:dialog(npc, '아, 이거 내 소개가 늦었군. 나는 저 표신궁에 얽힌 수수께끼의 역사들을 연구하는 역사가라오. 대장군이 왜 사람들을 죽였는가, 폐허가 되다시피한 표신궁 주변과는 달리, 표신궁은 어떻게 아직도 건재할 수 있는 것인가... 이런 것들을 연구하고 있지.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0005 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0007::
        sel = me:list(npc, '음... 이런 곳을 돌아다니는 것을 보니 당신도 보통 사람은 아닌 것 같소만... 혹시 당신, 표신궁을 탐험할 생각으로 온거요?', { '예, 표신궁을 탐험하려고 왔습니다.', '어휴, 위험할 것 같아서 안되겠어요.' })
        if sel == nil or sel == 0 then 
            return
        end
        if sel == 2 then
            me:dialog(npc, '당신도 그렇게 생각하시오? 그럼 부디 몸조심하시게.', false, false)
            return
        end
        ::NPC_250_0008::
        btn = me:dialog(npc, '이거 반갑구려. 모험가를 만나는 게 얼마만인지... 실은 이 전에도 많은 모험가들이 표신궁에 들어갔었는데, 대부분 돌아오지 않았고 살아서 돌아온 몇몇 사람들은 입구에서 얼마 가지도 못하고 돌아왔지.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0009::
        btn = me:dialog(npc, '대체 표신궁 깊은 곳에는 무엇이 있는지...... 아무튼 그래서 말인데, 당신이 표신궁에 들어간다면 내 부탁 하나만 들어주지 않겠소? 사례는 후하게 드리겠소이다.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0008 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0010::
        sel = me:list(npc, '표신궁에는 여러가지 고문서들이 있을 거요. 이 고문서들을 내게 가져와 주면 내용을 해석해주는 것은 물론이고, 자료를 제공한 대가 또한 지불하겠소, 어떠시오?', { '예, 기꺼이 도와드리겠습니다.', '음, 그렇게 위험한 곳에는 가고 싶지 않군요.' })
        if sel == nil or sel == 0 then 
            return
        end
        if sel == 2 then
            me:dialog(npc, '하긴.. 보통 위험한 일이 아니니 다음에 기회가 되면 도와주시게.', false, false)
            return
        end
        ::NPC_250_0011::
        btn = me:dialog(npc, '고맙소! 당신이라면 왠지 꼭 성공할 것 같은 기분이 드는군, 행운을 빌겠소.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0012::
        btn = me:dialog(npc, '이것은 내 작은 성의요. 착수금조로 받아주시오.', false, false)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0011 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        q = me:start_quest(quest.QUEST_PYOSIN)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        q:step(1)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서1을 구하자.', 7, 1)
        me:money(me:money() + 5000)
        return
    end
    
    if q:completed() then
        me:dialog(npc, '요즘 어떻게 지내시나? 당신 덕분에 내 연구에 많은 진척이 있었소. 정말 고맙소.', false, false)
        return
    end
    
    local step = q:step()
    if step == 1 then
        ::NPC_250_0013::
        btn = me:dialog(npc, '오, 살아 돌아왔구려! 반갑소. 정말 반갑소. 잘 돌아오셨소이다.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0014::
        sel = me:list(npc, '그래, 표신궁에서 고문서를 찾아 오셨소?', { '동장군의 원령을 물리치고 고문서를 가져왔습니다.', '아뇨, 아직...' })
        if sel == nil or sel == 0 then 
            return
        end
        if sel == 2 then
            me:dialog(npc, '그렇군. 너무 서두르지는 않아도 되니 천천히 하시게나.', false, false)
            return
        end
        if not me:has_items(story_items[1].doc, 1) then
            me:dialog(npc, '아직 동장군을 쓰러뜨리지 못한건가? 하긴.. 동장군을 쓰러뜨리는 것은 무리일수도 있지.', false, false)
            return
        end
        ::NPC_250_0015::
        btn = me:dialog(npc, '동장군? ...오오! 과연... 역시 내 눈은 틀리지 않았구려. 동장군이라면 대장군의 충실한 수하인 오방장군 중 하나요. 그도 귀신이 되어 표신궁에 떠돌고 있는 모양이군...', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0016::
        btn = me:dialog(npc, '가까스로 살아나온 사람들에게서도 그런 이야기는 듣지 못했소이다. 하물며 동장군의 원령을 쓰러뜨리기까지 하다니! 오래간만에 대단한 모험가를 만난 것 같구려. 허허허...', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0015 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0017::
        btn = me:dialog(npc, '그럼 잠시만 기다려 보시오. 이 문서를 냉큼 해독해 보일 테니...', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0016 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        local code = me:exchange(
            { ['item'] = { [story_items[1].doc] = 1 } },
            { ['item'] = { [story_items[1].diary] = 1 }, ['money'] = story_items[1].money }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 동장군을 쓰러뜨리지 못한건가? 하긴.. 동장군을 쓰러뜨리는 것은 무리일수도 있지.', false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 대장군의일기1을 받을 수 없습니다.', false, true)
            return
        end
        q:step(2)
        ::NPC_250_0018::
        btn = me:dialog(npc, '......음, 다 됐소. 여기 번역한 문서가 있으니 한번 읽어나 보시오. 보아하니 이것은 먼 옜날 이 땅을 통일했던 유명한 대장군의 일기인 것 같구려. 아까 이야기했던 오방장군도 이 대장군의 수하들이라오.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0019::
        btn = me:dialog(npc, '그 대장군은 원래 선량한 사람이었으나 천하를 통일하고 권력을 잡은 후 수많은 사람을 학살한 폭군이 되고 말았다 하오. 권력이라는게 다 그런 법이긴 하지만...', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0018 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0020::
        btn = me:dialog(npc, '다 읽고 나서 말씀해 주시오. 부탁드릴 것도 있고 하니... 아, 그리고 이건 약속한 사례요. 약소하지만 받아 두시오. 앞으로 내 연구를 더 많이 도와주면 줄수록 더 많은 사례를 하겠소이다.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0019 
        end
        return
    end
    
    if step == 2 then
        ::NPC_250_0021::
        btn = me:dialog(npc, '다 읽어보셨소? 흠, 어떠시오? 고통받는 백성들을 위해 천하통일을 결심한 대장군의 웅대한 포부가 느껴지는 것 같지 않소?', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0022::
        btn = me:dialog(npc, '일기의 다음이 궁금해지는군. 혹시 표신궁에 또 갈 일이 있거든 이 일기의 다음편을 찾아봐 주셨으면 하오. 물론 사례는 하겠소.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0021 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0023::
        btn = me:dialog(npc, '동장군의 원령이 이 일기를 가지고 있었다고 하면, 다음 일기는 아마 오방장군 중 두번째 장군인 서장군의 원령이 가지고 있지 않을까 하는데...', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0022 
        end
        q:step(3)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서2를 구하자.', 7, 1)
        return
    end
    
    if step == 3 then
        if not me:has_items(story_items[2].doc, 1) then
            me:dialog(npc, '아직 오방장군중 하나인 서장군을 물리치지 못하신것 같군. 서장군을 물리친 후 일기를 구해와주시오.', false, false)
            return
        end
        ::NPC_250_0024::
        btn = me:dialog(npc, '오오, 두번째 고문서를 찾아오셨구려. 수고하셨소! 잠시만 기다리시오. 이번 일기는 어떤 내용일런지...', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0025::
        btn = me:dialog(npc, '......음...호오...... 과연, 과연... 그랬군. 허허허...', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0024 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        local code = me:exchange(
            { ['item'] = { [story_items[2].doc] = 1 } },
            { ['item'] = { [story_items[2].diary] = 1 }, ['money'] = story_items[2].money }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 서장군 관련 문서를 쓰러뜨리지 못한 건가?', false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 대장군의일기2를 받을 수 없습니다.', false, true)
            return
        end
        q:step(4)
        btn = me:dialog(npc, '...다 됐소. 한번 읽어 보시오. 역시 이 두번째 고문서는 대장군의 두번째 일기였소. 여기 약속한 사례도 받아가시구려.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        btn = me:dialog(npc, '다 읽고 나면 말씀해 주시오. 역시 또 부탁할 것이 있을 것 같구려. 허허...', false, false)
        return
    end
    
    if step == 4 then
        btn = me:dialog(npc, '다 읽어보셨소? 흠, 어떠시오?', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        btn = me:dialog(npc, '일기의 다음이 궁금해지는군. 남장군을 쓰러뜨리고 표신궁고문서3을 구해와 주시오. 물론 사례는 하겠소.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        q:step(5)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서3을 구하자.', 7, 1)
        return
    end
    
    if step == 5 then
        if not me:has_items(story_items[3].doc, 1) then
            me:dialog(npc, '아직 표신궁고문서3을 구하지 못했군. 남장군을 쓰러뜨리고 고문서를 얻어와주게.', false, false)
            return
        end
        ::NPC_250_0028::
        btn = me:dialog(npc, '세번째 고문서를 찾아오셨구려! 정말 수고하셨소! 당신은 여태껏 여기에 찾아온 사람들 중 가장 대단한 사람이오. 정말 놀랍소.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0029::
        btn = me:dialog(npc, '잠시만 기다려 주시오. 금방 번역해 보이겠소이다.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0028 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0030::
        btn = me:dialog(npc, '......음...그런 일이... 그 때는 그랬었군, 과연... 으음!', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0029 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        local code = me:exchange(
            { ['item'] = { [story_items[3].doc] = 1 } },
            { ['item'] = { [story_items[3].diary] = 1 }, ['money'] = story_items[3].money }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 남장군 관련 문서를 쓰러뜨리지 못한 건가?', false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 대장군의일기3을 받을 수 없습니다.', false, true)
            return
        end
        q:step(6)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서4를 구하자.', 7, 1)
        btn = me:dialog(npc, '...으음... 다 됐소. 예상한 대로 세번째 고문서는 대장군의 세번째 일기였소. 한번 읽어 보시오. 대장군이 전투에서 크게 패하고 도망치던 때에 쓴 일기인 것 같소.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        btn = me:dialog(npc, '다 읽고 나면 말씀해 주시오. 음...', false, false)
        return
    end
    
    if step == 6 then
        btn = me:dialog(npc, '다 읽어보셨소? 북장군을 쓰러뜨리고 표신궁고문서4를 구해와 주시오.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        q:step(7)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서4를 구하자.', 7, 1)
        return
    end
    
    if step == 7 then
        if not me:has_items(story_items[4].doc, 1) then
            me:dialog(npc, '아직 표신궁고문서4를 구하지 못했군. 북장군을 쓰러뜨리고 고문서를 얻어와주게.', false, false)
            return
        end
        ::NPC_250_0033::
        btn = me:dialog(npc, '네번째 고문서를 찾아오셨구려! 수고하셨소. 얼마나 기다렸는지 모르오. 자자, 어서 고문서를 보여주시오. 금방 해석해 드리리다.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0034::
        btn = me:dialog(npc, '......음...쯧쯧쯧...그랬군, 설마했더니...으음......', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0033 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        local code = me:exchange(
            { ['item'] = { [story_items[4].doc] = 1 } },
            { ['item'] = { [story_items[4].diary] = 1 }, ['money'] = story_items[4].money }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 북장군 관련 문서를 쓰러뜨리지 못한 건가?', false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 대장군의일기4를 받을 수 없습니다.', false, true)
            return
        end
        q:step(8)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서5를 구하자.', 7, 1)
        btn = me:dialog(npc, '......다 됐소. 한번 읽어 보시오. 매번 고맙소. 여기 사례도 받아가구려.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        btn = me:dialog(npc, '다 읽고 나면 말씀해 주시오...', false, false)
        return
    end
    
    if step == 8 then
        btn = me:dialog(npc, '다 읽어보셨소? 중앙장군을 쓰러뜨리고 표신궁고문서5를 구해와 주시오.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        q:step(9)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서5를 구하자.', 7, 1)
        return
    end
    
    if step == 9 then
        if not me:has_items(story_items[5].doc, 1) then
            me:dialog(npc, '아직 표신궁고문서5를 구하지 못했군. 중앙장군을 쓰러뜨리고 고문서를 얻어와주게.', false, false)
            return
        end
        ::NPC_250_0037::
        btn = me:dialog(npc, '오오, 다섯번째의 표신궁고문서를 찾아내셨구려! 수고하셨소. 그럼 이 고문서를 해독해 드릴 테니 잠시만 기다려 주시오.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0038::
        btn = me:dialog(npc, '...으음....음....!', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0037 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0039::
        btn = me:dialog(npc, '이런! ......호오...이런 일이 있었군......', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0038 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0040::
        btn = me:dialog(npc, '역시 그런 일이 있었기에... 그렇군, 과연...! 하지만 이게 정말일까? 으음...!!', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0039 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        local code = me:exchange(
            { ['item'] = { [story_items[5].doc] = 1 } },
            { ['item'] = { [story_items[5].diary] = 1 }, ['money'] = story_items[5].money }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 중앙장군 관련 문서를 쓰러뜨리지 못한 건가?', false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 대장군의일기5를 받을 수 없습니다.', false, true)
            return
        end
        q:step(10)
        btn = me:dialog(npc, '굉장히 흥미로운 내용이 쓰여있소. 하지만 너무 신비한 내용이라 믿어지지 않는구려. 어찌 되었건 한 번 읽어 보시오.', false, false)
        return
    end
    
    if step == 10 then
        ::NPC_250_0042::
        btn = me:dialog(npc, '다 읽으셨소? 보셨다시피... 대장군은... 악마와 계약한 것 인지도 모르겠소. 허, 이걸 믿어야 하나? 아무튼 그게 사실이라면 대장군이 그렇게 많은 사람들을 학살한 이유도 밝혀지는 셈이지...', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0043::
        btn = me:dialog(npc, '하지만 유감스럽게도 그것이 사실인지를 파악할 수 있는 다음 장은 찢겨져 있더군.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0042 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0044::
        btn = me:dialog(npc, '수고스럽겠지만 표신궁을 샅샅이 뒤져봐 주시오, 진실을 밝히기 위해선 대장군의 일기 다음 장... 그러니까 여섯번째의 표신궁고문서가 꼭 필요하지. 그것을 찾아와 주셨으면 하오.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0043 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0045::
        btn = me:dialog(npc, '오방장군은 모두 물리쳤으니... 아마 여섯번째 장은 표신궁어딘가에 숨겨져 있을 것이오. 구석구석까지 샅샅이 살펴봐 주셨으면 하오.', false, false)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0044 
        end
        q:step(11)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서6을 구하자.', 7, 1)
        return
    end
    
    if step == 11 then
        if not me:has_items(story_items[6].doc, 1) then
            me:dialog(npc, '여섯번째 고문서는 아마 표신궁 어딘가에 숨겨져 있는듯 하오. 구석구석 찾아봐 주시게.', false, false)
            return
        end
        ::NPC_250_0046::
        btn = me:dialog(npc, '오오, 숨겨진 여섯번째의 고문서를 찾아냈구려! 전에도 말했지만 당신 정말 대단한 사람이군. 수고하셨소. 그럼 이 고문서를 해독할테니 잠시만 기다려 주시오.', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0047::
        btn = me:dialog(npc, '......으음......?', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0046 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0048::
        btn = me:dialog(npc, '음... 이런! ......호오...이럴 수가......', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0047 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0049::
        btn = me:dialog(npc, '음... 과연, 그렇군! 오오, 신이시여...어찌 이런 일이...!', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0048 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        local code = me:exchange(
            { ['item'] = { [story_items[6].doc] = 1 } },
            { ['item'] = { [story_items[6].diary] = 1 }, ['money'] = story_items[6].money }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 해당 문서를 쓰러뜨리지 못한 건가?', false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 대장군의일기6을 받을 수 없습니다.', false, true)
            return
        end
        q:step(12)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁고문서를 모두 구하다.', 7, 1)
        ::NPC_250_0050::
        btn = me:dialog(npc, '이런 무서운 일이... 으음... 이게 정말이라면 우리는 엄청난 사실을 알아낸 것일지도 모르겠소. 일단 읽어나 보시오. 여기 있소...', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        btn = me:dialog(npc, '다 읽고 나면 이야기해 주시오. 음......', false, false)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0050 
        end
        return
    end
    
    if step == 12 then
        ::NPC_250_0052::
        btn = me:dialog(npc, '여섯번째 일기... 그러니까 마지막 일기인 것 같소. 아무튼 다 읽어보셨소?', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0053::
        btn = me:dialog(npc, '대장군이 천하통일 후 왜 흉악한 폭군이 되었는지... 왜 그렇게 죄없는 사람들을 학살했는지... 주인 없는 표신궁이 어떻게 아직도 건재할 수 있는지... 이제야 그 이유를 알 수 있게 됐구려...', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0052 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0054::
        btn = me:dialog(npc, '......', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0053 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0055::
        btn = me:dialog(npc, '......혹시!?', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0054 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0056::
        btn = me:dialog(npc, '이...이런!!! 큰일이오. 어쩌면... 어쩌면... 내 예상이 맞다면 정말 큰일이 일어날 것이오. 시간이 없소. 다신이 가지고 있는 일기장을 전부 조합해 보시오! 어서!', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0055 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        q:step(13)
        me:push_achievement(ACHIEVEMENT_PYOSIN, '일기장을 전부 조합하자.', 7, 1)
        if me:mkitem('대장군의일기겉표지', 1) then
            me:dialog(npc, '여기 책 겉표지도 있으니 같이 조합하시면 될 거요!', false, false)
        end
        return
    end
    
    if step == 13 then
        if not me:has_items('대장군의일기통합본', 1) then
            me:dialog(npc, '어서 조합해보시게. 겉표지와 같이 조합하면 될걸세!', false, false)
            return
        end
        ::NPC_250_0058::
        btn = me:dialog(npc, '...이런 일이... 결국 이런 일이...!', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0059::
        btn = me:dialog(npc, '...내가 예전에 조사했던 고문서가 있소. 어떤 날짜와 알 수 없는 숫자들이 적혀 있는 문서였는데...', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0058 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0060::
        btn = me:dialog(npc, '이 일기들이 쓰여진 시기와 대조해 보니, 그것은... 대장군이 부활하게 될 날짜가 적힌 고문서였던 거요!', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0059 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0061::
        sel = me:list(npc, '게다가, 그 날짜가 이제 얼마 남지 않았구려. 으으음......', { '그럴 수가...!' })
        if sel == nil or sel == 0 then 
            return
        end
        if sel ~= 1 then 
            return
        end
        ::NPC_250_0062::
        btn = me:dialog(npc, '대장군은 전에 말했듯이 천하통일 후 엄청난 폭군이 되었다오. 이 대장군이 영원한 생명을 얻어 다시 이 땅에 나타난다면 천하는 피바다가 될 거요... 이런 무서운 일이...', true, true)
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0063::
        btn = me:dialog(npc, '...아무튼 수고하셨소. 당신 덕분에 연구에 많은 진척이 있었소. 정말 고맙소. 이것은 이제껏 연구를 도와준 사례요. 재미있게 생긴 돌이지? 꽤 값어치가 나간다고 들었소만... 아마 오랜 경력을 가진 대장장이라면 이 돌을 단번에 알아볼 거요.', true, true)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0062 
        end
        if btn == DIALOG_RESULT.QUIT then 
            return
        end
        ::NPC_250_0064::
        btn = me:dialog(npc, '걱정되는구려. 내 연구가 성과를 보였으니 학자로서는 기쁘지만 이 땅이 또다시 피로 물들게 될 무서운 사실을 알게 되니, 착잡하기 이를 데가 없구려......', false, false)
        if btn == DIALOG_RESULT.PREV then 
            goto NPC_250_0063 
        end
        if me:mkitem('금강석조각', 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 금강석조각을 받을 수 없습니다.', false, true)
            return
        end
        
        q:complete()
        me:push_achievement(ACHIEVEMENT_PYOSIN, '표신궁역사가의 부탁을 들어주다.', 6, 1)
        me:money(me:money() + 100000)
        return
    end
end

local function pyosin_sell_docs(me, npc)
    local sell_items = {
        { name = '표신궁고문서1', price = 10000 },
        { name = '표신궁고문서2', price = 20000 },
        { name = '표신궁고문서3', price = 30000 },
        { name = '표신궁고문서4', price = 40000 },
        { name = '표신궁고문서5', price = 50000 },
        { name = '표신궁고문서6', price = 60000 },
    }
    local list_names = {}
    for i = 1, #sell_items do list_names[i] = sell_items[i].name end
    local btn, sel
    
    ::NPC_250_0065::
    btn = me:dialog(npc, '흠. 고문서를 팔러 왔다고?', true, true)
    if btn == DIALOG_RESULT.QUIT then 
        return
    end
    ::NPC_250_0066::
    btn = me:dialog(npc, '고문서는 권수가 높을수록 좀 더 높은가격으로 사고있네.', true, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_250_0065
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    sel = me:list(npc, '그래. 어떤 고문서를 팔겠는가?', list_names)
    if sel == nil or sel < 0 or sel > 5 then 
        return
    end
    local item = sell_items[sel + 1]
    local doc = item.name
    local price = item.price
    if not me:has_items(doc, 1) then
        me:dialog(npc, name_with(doc, '을', '를') .. ' 가지고 있지 않은데?', false, false)
        return
    end
    local confirm = me:list(npc, name_with(doc, '을', '를') .. ' ' .. price .. '전에 팔겠는가?', { '예, 팔겠습니다.', '아니오, 좀더 생각해볼게요.' })
    if confirm == nil then 
        return
    end
    if confirm ~= 0 then 
        return
    end
    local code = me:exchange(
        { ['item'] = { [doc] = 1 } },
        { ['money'] = price }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, name_with(doc, '을', '를') .. ' 가지고 있지 않은데?', false, false)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '금전을 받을 여유가 없군요.', false, false)
        return
    end
    me:dialog(npc, '고맙네. 다음에 또 생기면 언제든지 찾아와주게나.', false, false)
end

function NPC_250(me, npc)
    if me:promotion() < 1 then
        me:dialog(npc, '표신궁의 역사란 참 미묘하지..', false, false)
        return
    end
    local sel = me:list(npc, '......', { '표신궁에 관한 이야기를 한다.', '표신궁고문서를 판다.' })
    if sel == nil or sel == 0 then 
        return
    end
    if sel == 1 then
        pyosin_story(me, npc)
        return
    end
    if sel == 2 then
        pyosin_sell_docs(me, npc)
    end
end