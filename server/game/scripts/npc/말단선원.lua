local ITEM_GOOSE_EGG = '기러기알'
local MONEY_PER_EGG = 200

function NPC_145(me, npc)
    local quest = me:quest(QUEST_GOOSE_EGG)

    if quest == nil or quest:step() < 1 then
        ::NPC_145_0001::
        local sel, btn = me:list(npc, '아이고~ 이거 큰일이네! 어떡하지? 엉엉 울고싶다 정말.', { '무슨 일이세요?', '(그냥 지나친다)' }, false)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= 0 then
            return
        end
        ::NPC_145_0010::
        btn = me:dialog(npc, '이 근방은 우리 대양선단의 주요 식량 채취 장소 인데요, 해물, 해초, 과일 등이 풍부한 반면 항상 사나운 두꺼비들과 일본게들이 득시글거려 매번 이렇게 고생이랍니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_145_0001
        end
        ::NPC_145_0011::
        sel, btn = me:list(npc, '아 글쎄.. 이번엔 제가 애써 모아놓은 기러기알들을 그놈들이 다 훔쳐갔지 뭡니까요! 흑흑. 그걸 어떻게 되찾는다.', { '흠. 그거라면.. 제가 그놈들을 해치우고 되찾아 드리지요.', '어쩔 수 없죠 뭐..' }, false)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_145_0010
        end
        if sel ~= 0 then
            return
        end
        if quest == nil then
            quest = me:start_quest(QUEST_GOOSE_EGG)
            if quest == nil then
                me:dialog(npc, '퀘스트 시작 실패', false, true)
                return
            end
        end
        quest:step(1)
        me:dialog(npc, '헛! 정말입니까? 꼭 좀 부탁드립니다.\n알을 찾아오시면 개당 200전씩 사례하겠습니다.', false, false)
        return
    end

    local sel, btn = me:list(npc, '기러기알은 찾아오셨나요?', { '네', '아니오' }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel ~= 0 then
        return
    end

    local item = me:item(ITEM_GOOSE_EGG)
    local count = (item ~= nil) and item:count() or 0
    if count == 0 then
        me:dialog(npc, '기러기알 없습니다.', false, false)
        return
    end

    local pay = MONEY_PER_EGG * count
    local code = me:exchange(
        { ['item'] = { [ITEM_GOOSE_EGG] = count } },
        { ['money'] = pay }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '기러기알 없습니다.', false, false)
        return
    elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '금전을 더이상 받을 수 없습니다.', false, false)
        return
    end
    me:dialog(npc, string.format('기러기알 %d개에 대한 대가로 %d전을 드렸습니다.', count, pay), false, true)
end