local ACHIEVEMENT_DRAGON_KING = 19

local function run_puffer_general_accept(me, npc)
    local min_level = 40
    if me:level() < min_level then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local btn
    ::NPC_75_COS001::
    local sel, list_btn = me:list(npc, "자넨 뭍사람이 아닌가? 여기 용궁은 무얼하러 왔는지 궁금하네만 그보다 내 부탁좀 들어줄 수 있겠나?", { "물론이지오.", "싫어요" }, false)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel ~= 0 then
        return
    end
    ::NPC_75_COS002::
    btn = me:dialog(npc, "고맙네.. 사실 자네도 이곳 용궁에 대해서 소문을 들었을지 모르겠네만 요즘 용궁 주위에 심상찮은 분위기가 맴돌고 있다네...", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS001
    end
    ::NPC_75_COS003::
    btn = me:dialog(npc, "이에 내가 조사를 해 본 결과 용궁 8대 장군 중 하나인 복어장군이 반란에 가담하고 있다는 정보를 입수하게 되었다네..", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS002
    end
    ::NPC_75_COS004::
    btn = me:dialog(npc, "아마도 복어장군뿐 아니라 다른 이들도 관련 있는 것 같네만 아직 그것까지는 알 수가 없다네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS003
    end
    ::NPC_75_COS005::
    local sel2, list_btn2 = me:list(npc, "그래서 말인데 자네가 복어장군에게 정보를 캐내 올 수 있겠는가?", { "좋습니다. 제가 해보지요.", "너무 위험한 일인 것 같아서 못하겠어요." }, false)
    if list_btn2 == DIALOG_RESULT.QUIT then
        return
    end
    if sel2 == nil or sel2 ~= 0 then
        return
    end
    if not me:start_quest(QUEST_DRAGON_KING) then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(1)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "복어장군 생포 임무를 받다.", 7, 1)
    me:dialog(npc, "고맙네 어서 정보를 얻어오게나. 복홍이라면 복어장군이 어디있는지 알고 있을거라네.", false, true)
end

local function run_puffer_general_complete(me, npc)
    local item_document = "태자전음문서"
    if not me:has_items(item_document, 1) then
        me:dialog(npc, "어서 복어장군에게 정보를 얻어오게.", false, false)
        return
    end
    local btn
    ::NPC_75_COS006::
    btn = me:dialog(npc, "오오 정말로 이 문서를 가지고 왔구만.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_75_COS007::
    btn = me:dialog(npc, "내 자네에게 보답을 하겠네", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS006
    end
    ::NPC_75_COS008::
    local gift_sel, gift_btn = me:list(npc, "자네 능력치 중에서 어떤 것을 올리고 싶은가?", { "힘이요!", "지식이요!", "민첩성이요!" }, true)
    if gift_btn == DIALOG_RESULT.QUIT then
        return
    end
    if gift_btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS007
    end
    if gift_sel == nil or gift_sel < 0 or gift_sel > 2 then
        return
    end
    if gift_sel == 0 then
        me:base_str(me:base_str() + 1)
    elseif gift_sel == 1 then
        me:base_int(me:base_int() + 1)
    else
        me:base_dex(me:base_dex() + 1)
    end
    me:rmitem(item_document, 1, ITEM_DELETE_TYPE.GIVE)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest ~= nil then
        quest:step(2)
        quest:progress(0)
    end
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "복어장군 생포 임무 완료.", 7, 1)
    me:dialog(npc, "그럼 자네의 힘을 올려주도록 하겠네. 우옷!", false, true)
end

local function run_crab_general_accept(me, npc)
    local min_level = 50
    if me:level() < min_level then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local btn = me:dialog(npc, "자네가 지난번 복어장군에게서 구해준 태자전음문서에서 게장군 또한 연루되었다는 사실을 알게 되었네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local sel, list_btn = me:list(npc, "이번에는 내 직접 게장군을 심문해 볼 터이니 게장군을 생포해 줄 수 있겠나?", { "좋습니다.", "별로여..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(3)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "게장군 생포 임무를 받다.", 7, 1)
    ::NPC_75_COS011::
    btn = me:dialog(npc, "고맙네, 내 듣기로 게장군이 더 강한 힘을 얻기 위해 근래 탈피를 했다더군.\n\n아직은 몸이 덜 굳어서 활동을 극히 자제하고 있을 걸세.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_75_COS012::
    btn = me:dialog(npc, "하지만 게장군은 자기 수하들을 극히 아낀다 들었네.\n\n자네가 게장군의 수하들을 손봐주고 있으면 그도 참지 못하고 모습을 드러낼걸세.\n\n그럼 되도록 빨리 잡아오도록 하게", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS011
    end
end

local function run_crab_general_complete(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil or quest:progress() < 1 then
        me:dialog(npc, "아직 게장군을 잡아오지 못한 모양이군..", false, false)
        return
    end
    quest:step(4)
    quest:progress(0)
    me:mkitem("산해진미", 1)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "게장군 생포 임무 완료.", 7, 1)
    local btn
    ::NPC_75_COS013::
    btn = me:dialog(npc, "고맙네, 내 산해진미를 줄 터이니 한번 맛이나 보시게나.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_75_COS014::
    btn = me:dialog(npc, "이건 아주 몸에 좋은 음식이니 혼자 몰래 들게나....하하하", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS013
    end
end

local function run_octopus_general_accept(me, npc)
    local min_level = 60
    if me:level() < min_level then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local btn = me:dialog(npc, "지난번 자네의 도움으로 많은 정보를 얻게 되었네.\n\n예상보다 많은 자들이 이번 반란에 연루 되었더군.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local sel, list_btn = me:list(npc, "그래서 말인데, 자네가 이번에는 문어장군을 좀 잡아들일 수 있겠나?", { "걱정 마십시오!!!.", "별로 안 내키는데요..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(5)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "문어장군 생포 임무를 받다", 7, 1)
    me:dialog(npc, "고맙네. 문어장군은 생김새가 산적같이 생긴 자인데...\n\n매우 포악하니 조심해야 하네", false, true)
end

local function run_octopus_general_complete(me, npc)
    if me:quest(QUEST_DRAGON_KING) == nil or me:quest(QUEST_DRAGON_KING):progress() < 1 then
        me:dialog(npc, "아직 문어장군을 잡아오지 못했군..", false, false)
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    local sel, list_btn = me:list(npc, "문어장군은 어디에 있고 왜 혼자 있나?", { "문어장군은 자신의 결백을 증명해 보이고 싶다고", "자결을 하였습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end
    sel, list_btn = me:list(npc, "뭐라? 자결을?", { "그렇습니다.", "용왕님에게 의심을 받을 바에야 죽어서", "결백을 증명해 보이는 길을 택하겠다고 하며", "자결을 하며 이 다문창을 전해주라 하였습니다" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 3 then
        return
    end
    sel, list_btn = me:list(npc, "음..그래? 안타깝네 그려.. 허나, 죽음으로 밝혀지는 건 아무것도 없네.", { "..." }, true)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "이 다문창은 문어장군이 가장 아끼던 무기인데, 자네가 보관해 두게나.", { "고맙습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    quest:step(6)
    quest:progress(0)
    me:mkitem("다문창", 1)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "문어장군 생포 임무 완료.", 7, 1)
end

local SEAHORSE_SOLDIER_MAX = 500

local function run_seahorse_accept(me, npc)
    local min_level = 70
    if me:level() < min_level then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "오...어서오게. 내가 자네를 얼마나 찾았는지 자넨 알고 있었나?.", { "무슨 급한 일이 있으신지요?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local btn = me:dialog(npc, "이번 반란에 대한 새로운 정보를 얻었네.\n\n그동안 자네가 내 부탁을 아주 잘 그리고 훌륭하게 들어 주었기 때문에 이번에도 자네의 도움을 좀 얻어볼까 해서야.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    sel, list_btn = me:list(npc, "어떤가. 또다시 내 부탁을 좀 들어주겠나?", { "물론이지요.", "죄송합니다만..바뻐서..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(7)
    quest:progress(0)
    quest:param("")
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "해마장군의 생포 임무를 받다", 7, 1)
    btn = me:dialog(npc, "고맙네. 자네가 도와 준다면 안심이라네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "이번에 얻은 정보는 용궁 8대장군 중 해마장군이라는 자가 있는데\n이 자가 반란군의 주력 부대를 생산하는 역할을 담당하고 있다 하는군.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "그래서 이자와 주력부대를 소탕하려고 하는데\n그에 앞서 자네가 내부로 침투해서 혼란을 야기시켜 주면 좋겠네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc, "가능하다면 해마장군을 사로잡거나 쓰러뜨려주면 더욱 좋겠네.\n\n그럼 부탁하네.", true, true)
end

local function run_seahorse_complete(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    local king_killed = (quest:param() == "1")
    local soldier_count = quest:progress()
    local is_taoist = (me:class() == CLASS.POET)
    if not king_killed then
        me:dialog(npc, "아직 해마장군을 쓰러트리지 못한 모양이군.", false, false)
        return
    end
    if not is_taoist and soldier_count < SEAHORSE_SOLDIER_MAX then
        me:dialog(npc, "결국 해마장군을 쓰러트렸군. 고맙네. 이제 그의 주력부대를 소탕해주게.", false, false)
        return
    end
    quest:step(8)
    quest:progress(0)
    quest:param("")
    me:mkitem("진주반지", 1)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "해마장군 생포 임무 완료.", 7, 1)
    local btn
    ::NPC_75_COS030::
    btn = me:dialog(npc, "지금 해마장군 소속 병사들이 대 혼란에 빠졌다고 들었네.\n\n이 모든 것이 자네 덕분이야.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_75_COS031::
    btn = me:dialog(npc, "지금이라면 해마장군 소속 병사들을 와해시키기 쉬울 듯 하네.\n\n정말 고맙네. 이것은 내 조그마한 성의니 받아두게", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS030
    end
end

local ITEM_MERMAID_DOC = "내통문서"
local ITEM_MERMAID_DOC_TRANSLATED = "번역된내통문서"
local ITEM_MERMAID_STAFF = "인어장군지팡이"

local function run_mermaid_accept(me, npc)
    local min_level = 99
    if me:level() < min_level then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local btn
    ::NPC_75_COS032::
    btn = me:dialog(npc, "어서오게.\n\n자네 덕분에 반란은 사전에 많이 진압을 했네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_75_COS033::
    btn = me:dialog(npc, "하지만 아직 그 주동자를 알아내지 못했고, 지금은 교착 상태에 빠져 있네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS032
    end
    ::NPC_75_COS034::
    btn = me:dialog(npc, "곰곰히 생각해 보니, 나는 지금껏 게장군의 말만 믿고 반란에 관한 일을 진행해 오지 않았나.\n\n지금 와서 드는 생각이네만, 게장군의 말을 전부 믿을 수 없을 것 같네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS033
    end
    ::NPC_75_COS035::
    btn = me:dialog(npc, "그래서 말인데, 게장군도 인어장군에게 설득을 당해 이번 반란에 가담하게 되었다고 했으니, 인어장군은 뭔가 더 많은 것을 알고 있을 것이라 생각하네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_75_COS034
    end
    local sel, list_btn = me:list(npc, "그러니 이번에 자네가 인어장군에게 정보를 좀 얻어 올 수 있겠나?", { "그 정도야 가뿐하지요.", "이번만은 좀 힘들 것 같군요..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(9)
    quest:progress(0)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "인어장군 생포 임무를 받다.", 7, 1)
    me:dialog(npc, "역시 자네는 시원 시원 하구만, 이번 일을 잘 해 오면 성의의 표시를 하지.\n\n잘 부탁하네", false, true)
end

local function run_mermaid_complete(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil or quest:progress() < 1 then
        me:dialog(npc, "아직 인어장군을 잡지 못하였군..", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "그래, 인어장군 일은 어떻게 되었나?", { "인어장군을 사로잡아 물어보니", "반란의 죄를 인정하였습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end
    sel, list_btn = me:list(npc, "그래? 음..역시.. 그래서 지금 인어장군은 어디에 있나?", { "그게, 인어장군 어미가 많이 아프셔서", "임종을 지켜보기를 간곡히 부탁하길래", "여기 이 내통문서를 받고 풀어주었습니다.", "어미일이 끝나는대로 용왕님을 찾아와서", "죄를 달게 받는다고 했습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 4 then
        return
    end
    sel, list_btn = me:list(npc, "음.. 그렇군, 그 내통문서를 이리 줘 보게", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    if not me:has_items(ITEM_MERMAID_DOC, 1) then
        me:dialog(npc, "내통문서가 없지 않은가?", false, false)
        return
    end
    sel, list_btn = me:list(npc, "음.. 아무래도 자네가 속은 듯 하네.", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "이 문서를 도저히 알아 볼 수 없는 언어로 되어 있네 그려, 동방언어가 아니라 아마 서방언어인 듯 싶은데..", { "..." }, true)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "아무래도 인어장군은 자네나 내가 이 문서의 내용을 알아 볼 수 없다는 것을\n짐작하고 쉽게 내어 준 것 같네.", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "아마도 어미의 일도 거짓일테지. 인어장군, 역시 간사하구나, 저런 저런...", { "아니, 그게 정말이옵니까? 이리 줘 보십시오." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, " ", { "This is the secret document....", "으허..이게 도대체 무슨 말인지..", "제 실수로 인어장군을 그냥 놓아 주었군요.", "아마도 인어장군은 지금쯤", "어디론가 모습을 감추었겠네요." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 4 then
        return
    end
    sel, list_btn = me:list(npc, " ", { "제 부주의로 인한 것이니,", "제가 반드시 이 문서를 번역해 오겠습니다.", "이것도 분명 언어이니", "누군가가 번역할 수 있겠지요" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 3 then
        return
    end
    sel, list_btn = me:list(npc, "그래, 자네만 믿고 기다리겠네. 아마 중요한 정보가 담겨 있을 것이야.", { "최선을 다하겠습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    quest:step(10)
    quest:progress(0)
end

local function run_mermaid_turnin(me, npc)
    if not me:has_items(ITEM_MERMAID_DOC_TRANSLATED, 1) then
        me:dialog(npc, "아직 내통문서를 번역하지 못했나 보군..", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "오.. 내통문서를 번역해 왔나 보군.", { "네, 여기 있습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    me:rmitem(ITEM_MERMAID_DOC_TRANSLATED, 1, ITEM_DELETE_TYPE.GIVE)
    quest:step(11)
    me:mkitem(ITEM_MERMAID_STAFF, 1)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "인어장군 생포 임무 완료.", 7, 1)
    me:dialog(npc, "고맙네. 이 지팡이를 받아 주게. 그리고 앞으로도 자주 용궁에 들러 주게.", false, true)
end

local SHARK_ANGER_SEC = 10

local function run_shark_accept(me, npc)
    local min_level = 99
    if me:level() < min_level then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "오. 마침 적절할 때 잘 왔네.", { "무슨 일이신가요?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local btn
    btn = me:dialog(npc, "자네가 구해다 준 내통문서 덕분에 누가 반역에 가담했는지 알게 되었다네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::SHARK_COS041::
    btn = me:dialog(npc, "그런데 알고보니 문어장군은 정말로 반역에 가담하지 않았더군.\n\n내 문어장군 같이 훌륭한 충신을 의심하다니....아...문어장군.....", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::SHARK_COS042::
    btn = me:dialog(npc, "문어장군. 내 조그맣게 속죄하는 뜻에서 그대의 의지를 용궁 대대로 전해지게 하겠네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto SHARK_COS041
    end
    ::SHARK_COS043::
    btn = me:dialog(npc, "그나저나 내통문서에 적힌대로 상어장군에게 첩자를 심어놓았는데\n적들이 반란을 시도하기 위한 준비를 거의 다 끝냈다는 사실을 알아냈다네.\n\n지금 적들의 사기가 하늘을 찌르고 있다고 하더군.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto SHARK_COS042
    end
    sel, list_btn = me:list(npc, "그래서 자네가 적들의 기세를 좀 꺽어주지 않겠는가?", { "매우 쉬운 일이군요.", "저 개인이 무슨 힘이 있어서...." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(12)
    quest:progress(0)
    quest:param("0")
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "상어장군의 생포 임무를 받다. [1/2]", 7, 1)
    ::SHARK_COS045::
    btn = me:dialog(npc, "정말 고맙네.\n\n마침 상어장군이 돌격대장을 함께 역임하고 있다고 하니, 그자를 제거하면 적의 기세가 한풀 꺽일걸세,", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "상어장군을 제거하고 돌아오면 그대에게 험난한 세상을 해어 나가는데 도움이 될 힘을 주겠네.", true, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto SHARK_COS045
    end
end

local function run_shark_report(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil or quest:progress() < 1 then
        me:dialog(npc, "아직 상어장군을 잡아오지 못한 모양이군..", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "그래 상어장군을 잡아왔는가?", { "예. 여기있습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "아니, 저자는 상어장군이 아니지 않은가?", { "예? 분명히 저자가", "자신의 입으로 상어장군이라고 했는데..." }, true)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end
    local btn = me:dialog(npc, "저자는 상어장군의 심복인 상어장수라네. 자네가 저자의 농간에 속았구만.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    sel, list_btn = me:list(npc, " ", { "이런. 제가 다시 가서 잡아오도록 하겠습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    quest:step(13)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "상어장군의 생포 임무를 받다. [2/2]", 7, 1)
    me:dialog(npc, "그래주겠나? 그럼 다시한번 부탁하네.", false, true)
end

local function run_shark_complete(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    local progress = quest:progress()
    if progress == 2 then
        local btn
        ::SHARK_COS047::
        btn = me:dialog(npc, "자네마저 나에 기대에 배신하다니....믿을 수 없군. 믿을수가 없어...", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "믿는 도끼에 발등 찍힌다더니 자네가 딱 그꼴이지 않은가!!!\n\n지금은 자네를 보기도 싫으니 어서 사라지게!!!", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto SHARK_COS047
        end
        quest:param(tostring(now() + SHARK_ANGER_SEC))
        quest:progress(0)
        return
    end
    local anger_end = tonumber(quest:param() or "")
    if anger_end and now() < anger_end then
        me:dialog(npc, "아직 내 화가 다 안풀렸으니 다음에 다시 오게!!!", false, true)
        return
    end
    if progress < 1 then
        me:dialog(npc, "다시 상어장군을 잡아오게나. 이번에도 속지 않게 조심하게.", false, false)
        return
    end
    local btn
    ::SHARK_COS049::
    btn = me:dialog(npc, "자네가 상어장군을 잡아올 때 상어장군이 한 말이 전 병사들에게 퍼져서 사기가 뚝 떨어졌다는 제보가 들어왔다네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::SHARK_COS050::
    btn = me:dialog(npc, "자네에게 약속대로 힘을 부여하겠네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto SHARK_COS049
    end
    btn = me:dialog(npc, "그럼 다음에 또 보세나", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto SHARK_COS050
    end
    quest:step(14)
    quest:progress(0)
    quest:param("")
    me:base_hp(me:base_hp() + 500)
    me:base_mp(me:base_mp() + 250)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "상어장군 생포 임무 완료", 7, 1)
end

local ITEM_JELLYFISH_STRATEGY = "전략문서"
local ITEM_DRAGON_KING_RING = "용왕의반지"

local function run_jellyfish_accept(me, npc)
    local min_promotion = 1
    if me:promotion() < min_promotion then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "이거 번번히 신세를 져서 미안하구만.", { "또 무슨 일이 있으신가 보지요?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local btn
    ::JELLYFISH_COS053::
    btn = me:dialog(npc, "지금 우리 군대와 반란군의 군대가 한참 전쟁일 벌이고 있는데, 적의 주모자를 제거하면 우리에게 유리한 방향으로 이끌어 나갈 수 있을 것 같네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::JELLYFISH_COS054::
    btn = me:dialog(npc, "그런데 적군의 장수 중에서 해파리 장군이 그 정보를 알고 있다고 하더군", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto JELLYFISH_COS053
    end
    sel, list_btn = me:list(npc, "자네가 나서서 그 정보를 좀 알아와 줄 수 있는가?", { "물론입니다.", "별로 내키지 않는군요." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(15)
    quest:progress(0)
    quest:param("0")
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "해파리장군 생포 임무를 받다.", 7, 1)
    me:dialog(npc, "이거 번번히 정말 고맙네. 그럼 부탁하겠네.", false, true)
end

local function run_jellyfish_report(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil or quest:progress() < 1 then
        me:dialog(npc, "하직 해파리장군을 잡아오지 못한것 같군.", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "해파리장군을 잡아왔는가?", { "예" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "그럼 적의 전략문서는 어디 있는가?", { "미처 발견하지 못했습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "음. 아마도 해파리 장군이 잡히기 직전에 측근들에게 넘겨주었을 걸세.", { "..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "머저 해파리부하를 여러마리 잡고나서 해파리수하를 닥달해보면 겁을 집어먹고 줄 것일세.", { "그렇다면 제가 가서", "그 전략문서를 찾아오도록 하겠습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 1 then
        return
    end
    quest:step(16)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "전략문서를 얻자.", 7, 1)
    me:dialog(npc, "그래주겠다면 정말 고맙네.", false, true)
end

local function run_jellyfish_complete(me, npc)
    if not me:has_items(ITEM_JELLYFISH_STRATEGY, 1) then
        me:dialog(npc, "아직 전략문서를 구하지 못한 모양이군. 해파리수하와 해파리부하를 잡다보면 줄걸세.", false, false)
        return
    end
    local btn
    btn = me:dialog(npc, "오 문서를 가져왔군. 이리 주시게.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::JELLYFISH_COS061::
    local sel, list_btn = me:list(npc, "음....으으으........으윽...", { "앗! 무슨 일이십니까?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    btn = me:dialog(npc, "처...청의태자가 반란의 수괴였다니.....믿을 수 없어.....이럴수가....", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    sel, list_btn = me:list(npc, "가만히 기다리면 저절로 자신의 손에 들어갈 용왕의 자리가 그렇게도 탐이 냈단 말인가....아....", { "고정하시지요." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    btn = me:dialog(npc, "여하튼 자네에게 커다란 빚을 지는구만,", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::JELLYFISH_COS065::
    btn = me:dialog(npc, "내 자네에게 보답으로 용왕의반지를 주겠네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto JELLYFISH_COS065
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    me:rmitem(ITEM_JELLYFISH_STRATEGY, 1, ITEM_DELETE_TYPE.GIVE)
    quest:step(17)
    quest:progress(0)
    quest:param("")
    me:mkitem(ITEM_DRAGON_KING_RING, 1)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "해파리장군 생포 임무 완료.", 7, 1)
end

local ITEM_ORB = "여의주"
local ITEM_DRAGON_KING_HELM = "용왕의투구"

local function run_crown_prince_accept(me, npc)
    local min_promotion = 2
    if me:promotion() < min_promotion then
        me:dialog(npc, "아직 나를 돕기엔 많이 부족한 것 같군..", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "으음. 이제서야 겨우 충격에서 회복되는군.", { "괜찮으십니까?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local btn
    ::CP_COS067::
    btn = me:dialog(npc, "좀 나아진것 같기는 하구만. 신경 써 주어서 고맙다네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::CP_COS068::
    btn = me:dialog(npc, "청의태자가 아직 어릴 때 양자로 들여서 여태까지 그렇게 사랑을 쏟아부었는데 이럴줄은 몰랐네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS067
    end
    ::CP_COS069::
    btn = me:dialog(npc, "하여간 충격에서 벗어나고 나니까 이제는 화가 치밀어 오르는구만.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS068
    end
    ::CP_COS070::
    btn = me:dialog(npc, "내 처음에는 그놈의 정에 이끌려서 청의태자가 도망가게 내버려 둘까 생각도 해 보았네만,", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS069
    end
    ::CP_COS071::
    btn = me:dialog(npc, "얼마전에 인어장군이 심어놓은 첩자가 여의주를 훔쳐서 청의태자에게 도망가 버렸다네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS070
    end
    ::CP_COS072::
    btn = me:dialog(npc, "이제는 용서하고 싶어도 용서할 수 없는 지경에 이르렀지.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS071
    end
    sel, list_btn = me:list(npc, "그래서 말인데 자네가 도와주던 김에 확실히 좀 도와주겠는가?", { "끝을 보는 게 옳겠지요.", "남의 가족사에는 끼고 싶지 않아요." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    quest:step(18)
    quest:progress(0)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "청의태자 생포 임무를 받다. [1/2]", 7, 1)
    ::CP_COS074::
    btn = me:dialog(npc, "정말 고맙네. 고마워.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "가서 청의태자를 붙잡고 여의주를 되찾아 와 주시게", true, false)
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS074
    end
end

local function run_crown_prince_turnin_orb(me, npc)
    if not me:has_items(ITEM_ORB, 1) then
        me:dialog(npc, "어서 청의태자를 물리치고 여의주를 되찾아 와 주시게", false, false)
        return
    end
    local sel, list_btn = me:list(npc, "그래 청의태자는 어떻게 되었는가?", { "청의태자는 그만 여의주의 힘을" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, " ", { ",견디지 못하고 사라져 버렸습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "그래. 그렇다면 여의주를 나에게 되돌려 주게나.", { "예. 여기있습니다.", "아니. 드릴 수 없습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "앗. 이러수가.", { "왜 그러십니까?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "청의태자는 아직 죽은 것이 아니었다네.", { "헉. 그렇다면......" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "청의태자가 죽기 직전에 여의주의 힘을 피해서 잠시 아공간으로 사라졌던 모양이야.", { "그럴수가..." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, "어서 가서 그자를 잡아다 주게.", { "예. 알겠습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    me:rmitem(ITEM_ORB, 1, ITEM_DELETE_TYPE.GIVE)
    quest:step(19)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "청의태자 생포 임무를 받다. [1/2]", 7, 1)
end

local function run_crown_prince_complete(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        return
    end
    if quest:progress() < 2 then
        me:dialog(npc, "아직 청의태자를 잡아오지 못한 건가?", false, false)
        return
    end
    local btn
    local sel, list_btn = me:list(npc, "오오. 청의태자를 잡아왔는가", { "......" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, " ", { "...청의태자를 잡았으나 여의주를" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = me:list(npc, " ", { "사용하다가 사라져버렸습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    btn = me:dialog(npc, "그렇다면 둘이 함께 여의주의 힘을 사용한 모양이로군.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    sel, list_btn = me:list(npc, "둘이 나눠서 여의주의 힘을 받으면 버틸 수 있을 것이라고 믿었던 모양이군. 어리석구나...어리석어...", { "그런데 여의주의 힘이 무엇인가요?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    ::CP_COS088::
    btn = me:dialog(npc, "여의주. 그것은 우리 용궁에 있는 비보들을 한군데로 모을 수 있게 해 주는 힘을 가진 구슬이라네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::CP_COS089::
    btn = me:dialog(npc, "더 이상은 우리 용궁의 비밀이라 말해줄 수 없군.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS088
    end
    ::CP_COS090::
    btn = me:dialog(npc, "이제 반란이 자네의 덕분으로 무사히 진압이 되었네", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS089
    end
    btn = me:dialog(npc, "내 감사의 뜻으로 이 용왕의투구를 주겠네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto CP_COS090
    end
    quest:step(20)
    quest:progress(0)
    quest:complete()
    me:mkitem(ITEM_DRAGON_KING_HELM, 1)
    me:push_achievement(ACHIEVEMENT_DRAGON_KING, "용궁 8대 반란 임무를 완수하다.", 6, 1)
    me:dialog(npc, "언제든지 용궁의 은인으로 반갑게 맞이할 테니 종종 놀러 오게나.", false, true)
end

function NPC_75(me, npc)
    local quest = me:quest(QUEST_DRAGON_KING)
    if quest == nil then
        run_puffer_general_accept(me, npc)
        return
    end

    if quest:completed() then
        me:dialog(npc, me:name() .. "이군. 반란을 막아준걸 정말 고맙게 생각하고 있네.", true, false)
        return
    end

    local step = quest:step()
    if step == 1 then
        run_puffer_general_complete(me, npc)
        return
    end
    if step == 2 then
        run_crab_general_accept(me, npc)
        return
    end
    if step == 3 then
        run_crab_general_complete(me, npc)
        return
    end
    if step == 4 then
        run_octopus_general_accept(me, npc)
        return
    end
    if step == 5 then
        run_octopus_general_complete(me, npc)
        return
    end
    if step == 6 then
        run_seahorse_accept(me, npc)
        return
    end
    if step == 7 then
        run_seahorse_complete(me, npc)
        return
    end
    if step == 8 then
        run_mermaid_accept(me, npc)
        return
    end
    if step == 9 then
        run_mermaid_complete(me, npc)
        return
    end
    if step == 10 then
        run_mermaid_turnin(me, npc)
        return
    end
    if step == 11 then
        run_shark_accept(me, npc)
        return
    end
    if step == 12 then
        run_shark_report(me, npc)
        return
    end
    if step == 13 then
        run_shark_complete(me, npc)
        return
    end
    if step == 14 then
        run_jellyfish_accept(me, npc)
        return
    end
    if step == 15 then
        run_jellyfish_report(me, npc)
        return
    end
    if step == 16 then
        run_jellyfish_complete(me, npc)
        return
    end
    if step == 17 then
        run_crown_prince_accept(me, npc)
        return
    end
    if step == 18 then
        run_crown_prince_turnin_orb(me, npc)
        return
    end
    if step == 19 then
        run_crown_prince_complete(me, npc)
        return
    end

    me:dialog(npc, "잘 왔네. 다음 부탁이 있으면 그때 찾아오게나.", false, false)
end
