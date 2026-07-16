-- npc: 고구려선원
local quest = require('lib.quest')
local enum = require('lib.enum')

function NPC_169(me, npc)
    local q = me:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 10 then
        me:dialog(npc, "안녕하신가? 여행이란 참 즐거운 일이지.", false, false)
        return
    end
    if not me:has_items("내통문서", 1) then
        me:dialog(npc, "안녕하신가? 여행이란 참 즐거운 일이지.", false, false)
        return
    end

    local sel, list_btn = me:list(npc, "무슨 일인가?", { "혹시 이 문서에 적힌", "내용을 알아볼 수 있는지요?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 1 or sel > 2 then
        return
    end

    local btn
    ::NPC_169_0001::
    btn = me:dialog(npc, "글쎄 한 번 보도록 하지\n\n...음......으음......", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_169_0002::
    btn = me:dialog(npc, "내가 배를 타고 여기 저기를 많이 돌아봐서 아네만 이건 서방 언어일세.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_169_0001
    end

    sel, list_btn = me:list(npc, "우리 동방 언어와는 근본적으로 틀리지.", { "그럼 이것을 알아보실 수 있는지요?" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
        return
    end

    sel, list_btn = me:list(npc, "하하..물론 가능하지...헌데...", { "......" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
        return
    end

    ::NPC_169_0003::
    btn = me:dialog(npc, "허허..자네 내가 이것을 번역해 주는 대신 무엇인가 요구를 할 것 같아 잔뜩 긴장해 있구만,\n하긴 무리도 아니지.. 요즘 세상에 공으로 되는 일이 거의 없으니 말일세.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_169_0004::
    btn = me:dialog(npc, "하지만 난 틀려, 나는 한평생을 바다와 함께 살아온 바다 사나이라네.\n나에게는 먹을 음식과 입을 옷만 있으면 그 이상은 필요 없다네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_169_0003
    end
    ::NPC_169_0005::
    btn = me:dialog(npc, "이건 내가 조건 없이 번역을 해 주겠네. 그 문서를 이리 주게.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_169_0004
    end

    sel, list_btn = me:list(npc, " ", { "정말 고맙습니다.", "일본에 자주 왔다갔다 하는데,", "항상 말이 없어 매우 무뚝뚝하고", "무서우신 분인 줄 알았는데", "참 친절하신 분이시군요." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 1 or sel > 5 then
        return
    end

    local code = me:exchange(
        { ['item'] = { ["내통문서"] = 1 } },
        { ['item'] = { ["번역된내통문서"] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "안녕하신가? 여행이란 참 즐거운 일이지.", false, false)
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 번역된내통문서를 받을 수 없습니다.", false, false)
        return
    end
    me:push_achievement(19, "번역된내통문서를 얻다.", 7, 1)
    me:dialog(npc, "하하.. 앞으로 자주 인사하세나.", false, false)
end