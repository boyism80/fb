-- npc: 금고주인
local quest = require('lib.quest')
local enum = require('lib.enum')

local ACHIEVEMENT_ACCEPT = 27
local ACHIEVEMENT_DONE = 27
local ITEM_OLD_SAFE = '낡은금고'
local ITEM_OLD_ENVELOPE = '낡은봉투'
local ITEM_OLD_LETTER = '낡은연애편지'
local ITEM_BLACK_DAGGER = '흑장단검'
local CURSE_BUFF_SEC = 1250
local CURSE_AC_PENALTY = 49

local function run_intro_and_accept(me, npc)
    local btn
    ::NPC_172_0001::
    btn = me:dialog(npc, '으이구... 이 금고를 어떻게 연다? 열쇠가 없으니 도무지 열 방법이 없구만! 이를 어쩌지? 이를 어쩐다?', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_172_0002::
    local sel, btn = me:list(npc, '이 금고를 대체 어쩌면 좋지', { '무슨 일이라도 있나요?', '신경쓰지 말자.' }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0001
    end
    if sel == nil or sel ~= 1 then
        return
    end
    ::NPC_172_0003::
    btn = me:dialog(npc, '아 글쎄, 이 금고를 열어야 하는데 열쇠를 잃어 버렸지 뭐야. 내가 어렸을 때 나중에 커서 열어 보겠다고 잠근 금고인데, 막상 열려고 보니 열쇠가 없네.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_172_0004::
    sel, btn = me:list(npc, '뭐 방법이 없을까?', { '열지 못하면 그냥 부수는 건 어때요?', '그냥 포기하세요...' }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0003
    end
    if sel == nil or sel ~= 1 then
        return
    end
    ::NPC_172_0005::
    btn = me:dialog(npc, '뭐? 부순다고? 맞아! 그런 방법이 있었군! 내가 왜 그 생각을 못했지? 으하하하!', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_172_0006::
    btn = me:dialog(npc, '..............', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0005
    end
    ::NPC_172_0007::
    sel, btn = me:list(npc, '...그런데 어떻게 부수지?', { '이리 주세요. 제가 부숴 드리죠.', '몰라요. 알아서 하세요.' }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0006
    end
    if sel == nil or sel ~= 1 then
        return
    end
    ::NPC_172_0008::
    btn = me:dialog(npc, '으음...! 음, 뭐, 좋아. 자네가 부숴 주겠다고? 그래. 자네한테 맡기지. 성공하면 보수도 주겠어.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_172_0009::
    btn = me:dialog(npc, '단, 한가지 약속해줘야 할 게 있어!', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0008
    end
    ::NPC_172_0010::
    sel, btn = me:list(npc, '이걸 열면 안에서 봉투가 하나 나올 텐데, 절대 그 봉투를 열어봐선 안돼! 약속할 수 있겠나?', { '예, 절대 안 열어보고 그대로 가져다 드리지요.', '에이, 그게 뭐길래... 그냥 열어보면 안되나요?' }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0009
    end
    if sel == nil or sel ~= 1 then
        return
    end
    ::NPC_172_0011::
    btn = me:dialog(npc, '좋아! 여기 금고가 있네. 알아서 부순 다음 안에 들어있는 봉투만 내게 주면 돼.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, '그리고! 다시 한 번 말해두지만 안에 있는 봉투를 절대 열어봐서는 안 돼. 약속했지? 좋아, 수고하게.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local q = me:start_quest(quest.QUEST_STRONGBOX)
    if q == nil then
        return
    end
    q:step(1)
    me:push_achievement(ACHIEVEMENT_ACCEPT, '금고주인의 부탁을 들어주자.', 7, 1)
    me:mkitem(ITEM_OLD_SAFE, 1)
end

local function run_turn_in(me, npc)
    local code = me:exchange(
        { ['item'] = { [ITEM_OLD_ENVELOPE] = 1 } },
        { ['item'] = { [ITEM_BLACK_DAGGER] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.OK then
        local q = me:quest(quest.QUEST_STRONGBOX)
        if q then
            q:complete()
        end
        me:push_achievement(ACHIEVEMENT_DONE, '금고주인의 부탁을 들어주었다.', 6, 1)
        me:dialog(npc, '오! 봉투를 가져왔군. 수고했어. 설마했는데 정말 안 열어봤군. 좋았어! 내 아끼던 거지만 상으로 이 흑장단검을 주지. 중국에서 가져온 귀한 칼이야. 그럼 잘 가게!', { prev = false, next = true })
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 흑장단검을 받을 수 없습니다. 자리 좀 비우고 다시 오세요.', { prev = false, next = false })
        return
    end
    if me:has_items(ITEM_OLD_LETTER, 1) then
        local sel, btn = me:list(npc, '아직 멀었나?', { '아뇨, 다 됐습니다.', '예, 아직 멀었어요.' }, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            local btn = me:dialog(npc, '이...이놈! 내가 그렇게 말했는데 결국 봉투를 열어봤구나! 나쁜 놈! 약속을 어기다니! 에잇! 말한대로 보수는 없다!', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:rmitem(ITEM_OLD_LETTER, 1, ITEM_DELETE_TYPE.GIVE)
            me:buff('금고주인의저주', CURSE_BUFF_SEC)
            local dr = me:damage_derate()
            me:damage_derate(dr + CURSE_AC_PENALTY)
            local q = me:quest(quest.QUEST_STRONGBOX)
            if q then
                q:step(2)
            end
        elseif sel == 2 then
            me:dialog(npc, '그런가? 그럼 서둘러주게.', { prev = false, next = false })
        end
        return
    end
    me:dialog(npc, '아직 멀었나 보군..', { prev = false, next = false })
end

local function run_cursed_forgiveness(me, npc)
    local q = me:quest(quest.QUEST_STRONGBOX)
    if q == nil then
        return
    end
    me:dialog(npc, '.....', { prev = false, next = true })
    local sel, btn = me:list(npc, '에이! 네놈만 보면 화가 난다. 설마 했지만 그래도 믿었는데, 보란 듯이 약속을 어기다니! 저리 가! 꼴도 보기 싫다!', { '......', '죄송합니다. 호기심에 그만...' }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end
    
    local btn
    ::NPC_172_0016::
    btn = me:dialog(npc, '분명히 봉투를 열지 말라고 했는데도! 내가 몇 번이나 말했는데도 그걸 열다니!\n\n에이! 못된 놈 같으니라고!', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_172_0017::
    btn = me:dialog(npc, '쯧...\n\n\n...하지만 그걸 열기 위해 저 멀리 중국까지 다녀오며 고생 한 것도 사실이니, 내 그 사정은 좀 봐 주지.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0016
    end
    ::NPC_172_0018::
    sel, btn = me:list(npc, '대신! 자네가 읽은 그 편지 내용은 아무에게도 말하지 말 것! 어때, 약속할 수 있겠나?', { '예! 물론입니다. 아무렇게도 말하지 않겠어요.', '그건 좀...' }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0017
    end
    if sel == nil or sel ~= 1 then
        me:dialog(npc, '반성의 여지가 없군! 얘기는 끝일세!', { prev = false, next = false })
        return
    end

    q:complete()
    me:mkitem(ITEM_BLACK_DAGGER, 1)
    me:push_achievement(ACHIEVEMENT_DONE, '금고주인의 부탁을 들어주었다.', 6, 1)
    ::NPC_172_0019::
    btn = me:dialog(npc, '좋아. 그럼 자네가 노력한 것을 봐서 보수를 주지. 여기 흑장단검을 가져가게. 중국에서 가져온 귀한 물건이야.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_172_0020::
    btn = me:dialog(npc, '그럼 잘 가게. 편지 내용은 아무에게도 말하지 않겠다고 약속한 것도 잊지 말고!', { prev = true, next = true })
    if btn == DIALOG_RESULT.PREV then
        goto NPC_172_0019
    end
end

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_STRONGBOX)
        if q == nil then
            run_intro_and_accept(me, npc)
            return
        end
        if q:completed() then
            me:dialog(npc, '잘 지내시나? 저번엔 고마웠네.', { prev = false, next = true })
            return
        end
        if q:step() == 1 then
            run_turn_in(me, npc)
            return
        end
        if q:step() == 2 then
            run_cursed_forgiveness(me, npc)
        end
    end
}
