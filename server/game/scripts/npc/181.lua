-- npc: 안도
local quest = require('lib.quest')

function NPC_181(me, npc)
    local BURY_COUNT = 5
    local ACHIEVEMENT_GHOST = 29
    local q = me:quest(quest.QUEST_GHOST)
    local btn, sel

    if q == nil then
        ::NPC_181_0001::
        btn = me:dialog(npc, '혹시 자네도 이 이글거리는 기운이 보이는가? 집 전체에 귀기가 서려서 잠을 잘 수가 없군. 집값이 싸서 샀더니만 이런 귀신 붙은 집일 줄 몰랐어.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_181_0002::
        btn = me:dialog(npc, '지나가던 스님께 여쭤 봤더니 부적을 방 구석구석 바닥에 묻으면 귀신을 퇴치할 수 있다고 하더군.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_181_0001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_181_0003::
        sel = me:list(npc, '그래서 조금 전까지는 부적을 묻고 있는 중이였는데, 나 혼자서는 안되겠더구만. 보다시피 나 혼자 살기에는 집이 좀 넓어서 말이야.', { '제가 좀 도와 드리죠.', '열심히 붙이세요. 전 나가봐야겠네요.' })
        if sel == nil or sel ~= 1 then
            if sel == 2 then
                me:dialog(npc, '아이고. 이 많은 부적을 언제 다 묻는담...', { prev = false, next = true })
            end
            return
        end
        ::NPC_181_0004::
        btn = me:dialog(npc, '그래 주겠는가? 고맙네.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '이 부적을 가지고 집 구석구석을 살피다가... 허술해 보이는 부분이 있으면 바닥에 묻어주게나. 잘 부탁하네.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_181_0004
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local q = me:start_quest(quest.QUEST_GHOST)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
            return
        end
        q:step(1)
        q:progress(0)
        if me:mkitem('귀신퇴치부적', 5) == nil then
            me:dialog(npc, '소지품이 가득 차서 귀신퇴치부적을 받을 수 없습니다.', { prev = false, next = true })
            return
        end
        me:push_achievement(ACHIEVEMENT_GHOST, '안도의 부탁을 들어주자. [1/2]', 7, 6)
        return
    end

    if q:completed() then
        me:dialog(npc, '자네 덕분에 집에 귀기가 많이 사라졌어. 고맙네.', { prev = false, next = true })
        return
    end

    if q:step() == 1 then
        if q:progress() < BURY_COUNT then
            me:dialog(npc, '아직 부적을 다 묻지 못한 것 같군.', { prev = false, next = true })
            return
        end
        ::NPC_181_0006::
        btn = me:dialog(npc, '부적이 다 떨어졌다고? 큰일이군! 부적이 더 있어야겠는데...', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_181_0007::
        btn = me:dialog(npc, '그런데 이 부적은 예전에 중국에 갔을 때 귀신 쫓는 데에 능하다는 고승에게 받은 것이라 더 받으려면 중국까지 가야 할 판국이야. 이거 야단났군...', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_181_0006
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_181_0008::
        sel = me:list(npc, '이를 어쩐다...', { '제가 그 고승을 만나고 오겠습니다.', '아무래도 너무 힘들어서 안되겠네요. 전 이만...' })
        if sel == nil or sel ~= 1 then
            if sel == 2 then
                me:dialog(npc, '이를 어쩐다...', { prev = false, next = true })
            end
            return
        end
        ::NPC_181_0009::
        btn = me:dialog(npc, '정말 고맙네! 자넨 혹시 보살인가? 정말 친절하구만. 일이 끝나고 나면 내 꼭 사례하겠네.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '그렇지. 그 스님을 찾는다면 이름이라도 가르쳐 줘야겠군. 뭐라고 했더라? ...아, 그렇지.\n\n중국에 있다는 그 시님의 법명은 \'마타\'라고 했다네. 미안하지만 부적을 얻어 와 주게나. 부탁하네.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_181_0009
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q:step(2)
        return
    end

    if q:step() == 2 then
        me:dialog(npc, '아직 마타님을 만나지 못한 것 같군', { prev = false, next = true })
        return
    end

    if q:step() == 3 then
        if q:progress() < BURY_COUNT then
            me:dialog(npc, '아직 부적을 모두 묻지 못한 모양이군?', { prev = false, next = true })
            return
        end
        ::NPC_181_0011::
        btn = me:dialog(npc, '다 끝났군, 이젠 좀 살만하구만!', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_181_0012::
        btn = me:dialog(npc, '다 자네 덕분일세, 약속대로 사례를 해야겠는데... 뭐가 좋을까... 그렇지. 이걸 가져가게.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_181_0011
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_181_0013::
        btn = me:dialog(npc, '이건 내가 예전에 귀신을 쫓아 보려고 샀던 부적이야. 비슷하게 생겨서 효과가 있으려니 하고 샀더니만 귀신 쫓는 데에는 효과가 없고 그저 머리가 좀 맑아지더구만.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_181_0012
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '나처럼 머리 나쁜 사람한테는 별 쓸모가 없겠지만 자네한테는 쓸모가 있을 것 같으니 가져가주게나.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_181_0013
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if me:mkitem('명석부', 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 명석부를 받을 수 없습니다.', { prev = false, next = true })
            return
        end
        q:complete()
        me:push_achievement(ACHIEVEMENT_GHOST, '안도의 부탁을 들어주었다.', 7, 6)
        return
    end
end