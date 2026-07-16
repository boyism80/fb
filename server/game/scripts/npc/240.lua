-- npc: 보약의달인
local quest = require('lib.quest')
local enum = require('lib.enum')

function NPC_240(me, npc)
    local quest_mother = me:quest(quest.QUEST_SICK_CHILD)
    local q = me:quest(quest.QUEST_SAMJEONSIN)

    if quest_mother == nil or quest_mother:step() ~= 1 then
        me:dialog(npc, '장안성엔 어쩐일로 오셨나? 보약이라도 한 채 지어먹으러 오셨나?', { prev = true, next = true })
        return
    end

    if q == nil then
        local sel, lb = me:list(npc, '무슨 일인가? 급한 일이라도 있는가?', { '실은 아픈 아이가 있는데...', '아니오, 아무 일도 아닙니다.' }, { prev = false })
        if lb == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end
        ::NPC_240_0001::
        local btn = me:dialog(npc, '......음... 꼬마가 기력이 없고 말을 잘 하지 못한다... 그럴 때에는 삼전신보탕이 제격이지.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_240_0002::
        btn = me:dialog(npc, '만드는 것은 쉽다네, 하지만 지금 재료가 다 떨어졌군. 자네가 재료를 구해다 주면 삼전신보탕을 바로 만들어 주지.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_240_0001
        end
        ::NPC_240_0003::
        btn = me:dialog(npc, '삼전보신탕을 만들기 위해서는 감초, 녹용, 국광이 필요하지. 감초는 광동성 푸줏간에서, 녹용은 어디든 푸줏간에서 쉽게 구할 수 있고, 국광은 어딘가의 미궁에서 구할 수 있다고 하네.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_240_0002
        end
        btn = me:dialog(npc, '재료를 가져오기만 하면 만드는 건 금방이라네. 그럼 힘 내게나.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q = me:start_quest(quest.QUEST_SAMJEONSIN)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
            return
        end
        q:step(1)
        me:push_achievement(36, '삼전보신탕 재료를 구하자.', 7, 1)
        return
    end

    if q:completed() then
        me:dialog(npc, '....', { prev = true, next = true })
        return
    end

    if q:step() == 0 then
        local sel, lb = me:list(npc, '무슨 일인가? 급한 일이라도 있는가?', { '실은 아픈 아이가 있는데...', '아니오, 아무 일도 아닙니다.' }, { prev = false })
        if lb == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end
        ::NPC_240_0005::
        local btn = me:dialog(npc, '......음... 꼬마가 기력이 없고 말을 잘 하지 못한다... 그럴 때에는 삼전신보탕이 제격이지.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_240_0006::
        btn = me:dialog(npc, '만드는 것은 쉽다네, 하지만 지금 재료가 다 떨어졌군. 자네가 재료를 구해다 주면 삼전신보탕을 바로 만들어 주지.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_240_0005
        end
        ::NPC_240_0007::
        btn = me:dialog(npc, '삼전보신탕을 만들기 위해서는 감초, 녹용, 국광이 필요하지. 감초는 광동성 푸줏간에서, 녹용은 어디든 푸줏간에서 쉽게 구할 수 있고, 국광은 어딘가의 미궁에서 구할 수 있다고 하네.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_240_0006
        end
        btn = me:dialog(npc, '재료를 가져오기만 하면 만드는 건 금방이라네. 그럼 힘 내게나.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q:step(1)
        me:push_achievement(36, '삼전보신탕 재료를 구하자.', 7, 1)
        return
    end

    if q:step() == 1 then
        local materials = {['감초'] = 1, ['녹용'] = 1, ['국광'] = 1}
        if not me:has_items(materials) then
            me:dialog(npc, '삼전신보신탕을 만들기 위한 재료가 부족한 것 같네만. 녹용과 국광 그리고 감초가 있어야 제작이 가능하지.', { prev = false, next = true })
            return
        end
        ::NPC_240_0009::
        local btn = me:dialog(npc, '모든 재료를 가져왔군! 수고했네. 자, 그럼 삼전신보탕을 만들어 볼까? 잠시만 기다리게.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '자, 자 되었네. 이걸 먹으면 그 애도 건강해질 거야. 허허. 수고했네.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = materials },
            { ['item'] = { ['삼전신보탕'] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '삼전신보신탕을 만들기 위한 재료가 부족한 것 같네만. 녹용과 국광 그리고 감초가 있어야 제작이 가능하지.', { prev = false, next = true })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 삼전신보탕을 줄 수 없습니다.', { prev = false, next = true })
            return
        end
        q:step(2)
        me:push_achievement(36, '삼전보신탕을 만들었다.', 7, 1)
        return
    end

    me:dialog(npc, '....', { prev = true, next = true })
end