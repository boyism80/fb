local quest = require('lib.quest')
local server = require('lib.server')

function NPC_107(me, npc)

    local options = {'황비연 퀘스트'}
    local herb_quest = me:quest(quest.QUEST_HWANGBIYEON)
    if herb_quest ~= nil and herb_quest:step() == 2 then
        table.insert(options, '약초 퀘스트')
    end

    local selected = me:list(npc, '안녕하세요. 무엇을 도와드릴까요?', options)
    if selected == nil then
        return
    end
    
    if selected == 0 then
        dojaeyoung_nobidocument(me, npc)
    else
        dojaeyoung_herb_quest(me, npc)
    end
end

function dojaeyoung_nobidocument(me, npc)
    local selected = me:list(npc, '음... 뉘신지요? 제게 무슨 볼일이라도?', { '상해에서 물건을 전해달라는 부탁을 받고 왔습니다.', '아무 것도 아닙니다.' })
    if selected == nil then
        return
    end

    if selected == 1 then
        return
    end

    local q = me:quest(quest.QUEST_HWANGBIYEON)
    if q == nil then
        me:dialog(npc, '그건 아직 제게 맡기실 일이 없으신 것 같군요. 상해 쪽에서 부탁을 받고 오시면 말씀하세요.', false, true)
        return
    end
    if q:step() ~= 2 then
        me:dialog(npc, '그건 아직 제게 맡기실 일이 없으신 것 같군요. 상해 쪽에서 부탁을 받고 오시면 말씀하세요.', false, true)
        return
    end
    if not me:has_items('노비문서', 1) then
        me:dialog(npc, '노비문서를 가지고 오셨을 때 말씀해 주세요.', false, true)
        return
    end

    local sel, lb = me:list(npc, '아니, 이것은... 얼마 전에 풀어 줬던 황비연의 노비문서군요. 생각납니다. 여기서 일하던 노비였는데, 참 의협심이 강한 청년이였죠. 그는 잘 있나요?', { '예, 실은...' }, true)
    if lb == DIALOG_RESULT.QUIT or sel ~= 0 then
        return
    end
    ::NPC_107_0001::
    local btn = me:dialog(npc, '예? 뭐라고요? 그게 사실입니까? 하하... 이거 참, 뭐라고 말해야 할지...\n\n황비연... 그가 의적이 되어 탐관오리를 혼내주고 사람들을 돕고 있단 말이지요?', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_107_0010::
    btn = me:dialog(npc, '뭐라고 해야 할까... 참 대견스럽군요. 그가 어느날 갑자기 어려운 사람들을 돕고 싶다며 풀어달라고 할 때에는 반신반의했었지요.', true, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_107_0001
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    sel, lb = me:list(npc, '지금 보니 그 때 황비연을 저희 집안 노비 신세에서 풀어준 것이 잘한 일이었다는 생각이 드는군요.', { '.....' }, false)
    if lb == DIALOG_RESULT.QUIT or sel ~= 0 then
        return
    end
    ::NPC_107_0011::
    btn = me:dialog(npc, '이 보패를 그에게 전해주십시오. 이 문서를 제게 전해달라고 부탁했다는 그 사람이 아마 황비연 본인일 겁니다.', true, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_107_0010
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_107_0012::
    btn = me:dialog(npc, '아마, 아무리 의적이라고 해도 도둑직을 한 것이 부끄러워서 제게 직접 오지 못한 것이겠죠. 하하...\n\n그에게 전해 주십시오. 우린 모두 너를 아주 자랑스럽게 여기고 있다고... 하하하...', false, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_107_0011
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local code = me:exchange(
        { ['item'] = { ['노비문서'] = 1 } },
        { ['item'] = { ['보패'] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '노비문서를 가지고 있지 않으시군요.', false, true)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 보패를 받을 수 없습니다.', false, true)
        return
    end
    q:step(3)
    me:push_achievement(39, '보패를 가지고 다시 상해주민에게 가보자.', 7, 2)
end

function dojaeyoung_herb_quest(me, npc)
    local q = me:quest(quest.QUEST_DOJAEYOUNG_HERB)

    if q == nil then
        dojaeyoung_herb_start(me, npc)
        return
    end
    if q:step() == 0 then
        dojaeyoung_herb_start(me, npc)
        return
    end
    if q:step() == 1 then
        dojaeyoung_herb_turnin(me, npc)
        return
    end
    me:dialog(npc, '저번에 약초를 구해주셔서 정말 감사드립니다. 덕분에 아버님의 건강이 많이 좋아지셨습니다.', true, true)
end

function dojaeyoung_herb_start(me, npc)
    ::NPC_107_0020::
    local btn = me:dialog(npc, '안녕하세요 ' .. me:name() .. '님. 도삭산을 탐험하고 계시군요, 도삭산은 마법에 걸린 곳이라는 소문이있는데, 그 말처럼 이상한 여러가지 현상들이 일어나는 곳이랍니다.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_107_0021::
    btn = me:dialog(npc, '아무래도 이런 마법이 걸린 곳에서 보통 사람이 사는것은 무리인지, 저희 아버지도 요즘 건강이 안좋으시죠.', true, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_107_0020
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_107_0022::
    local sel, lb = me:list(npc, '의사의 말에 따르면 도삭산의 토끼들이 가지고 있는 약초를 달여서 지은 약을 드시면 차도가 있으실꺼라고 했는데...', { '제가 구해드리겠습니다.', '그렇군요..' }, false)
    if lb == DIALOG_RESULT.QUIT then
        return
    end
    if lb == DIALOG_RESULT.PREV then
        goto NPC_107_0021
    end
    if sel == 1 then
        me:dialog(npc, '그렇죠..이 약초들을 어떻게 구하면 좋으련지..', false, true)
        return
    end
    if sel ~= 0 then
        return
    end
    local q = me:start_quest(quest.QUEST_DOJAEYOUNG_HERB)
    if q == nil then
        me:dialog(npc, '퀘스트 시작 실패', false, true)
        return
    end
    q:step(1)
    me:push_achievement(9, '도재영의 부탁을 들어주자!', 7, 1)
    ::NPC_107_0025::
    btn = me:dialog(npc, '아.. 정말 감사합니다. ' .. me:name() .. '님은 마음도 넓으신 분이시군요.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_107_0026::
    btn = me:dialog(npc, '도삭산의 토끼들은 151층에서 200층에서 나온다고 합니다. 의사의 말로는 약초잎사귀 5개와 약초가지 1개가 있어야 한다고 하네요.\n\n그럼 잘 부탁드리겠습니다! 감사합니다!', true, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_107_0025
    end
end

function dojaeyoung_herb_turnin(me, npc)
    local q = me:quest(quest.QUEST_DOJAEYOUNG_HERB)
    if q == nil or q:step() ~= 1 then
        return
    end
    if not me:has_items({['약초잎사귀'] = 5, ['약초가지'] = 1}) then
        me:dialog(npc, '아직 약초를 구를 구하시지 못하셨군요. 의사의 말로는 약초잎사귀 5개와 약초가지 1개가 있어야 한다고 합니다. 부탁드립니다.', false, true)
        return
    end
    local btn = me:dialog(npc, '약초잎사귀와 약초가지를 구하셨군요! 감사합니다! 이것으로 저희 아버지가 건강을 되찾으시겠군요!', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local code = me:exchange(
        { ['item'] = { ['약초가지'] = 1, ['약초잎사귀'] = 5 } },
        { ['item'] = { ['강철의구두'] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '아직 약초를 구를 구하시지 못하셨군요. 의사의 말로는 약초잎사귀 5개와 약초가지 1개가 있어야 한다고 합니다. 부탁드립니다.', false, true)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 강철의구두를 줄 수 없습니다.', false, true)
        return
    end
    q:step(2)
    me:push_achievement(9, '도삭산 100층 퀘스트 완료', 7, 1)
    me:dialog(npc, '약소하지만 보답으로 강철의구두를 드리겠습니다. 도삭산을 돌아다니시면서 조금이라도 도움이 되길 바랍니다. 그럼..', false, true)
end