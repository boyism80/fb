function NPC_122(me, npc)
    local dq = me:quest(QUEST_DETECTIVE)
    if dq and not dq:completed() and dq:step() == 5 then
        local sel, list_btn = me:list(npc, "이런! 반갑지 않은 손님이 또 오셨구만!", {
            "조염에게 준 귀중한 문화재는 어디서 난거죠?",
            "망치에 대해서 이야기를 좀...",
            "지금 당장은 아무런 용무가 없습니다.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            ::NPC_122_0000::
            local btn = me:dialog(npc, "그건 모조품이라구! 아니, 그럼 내가 진짜를 줬을 줄 알았어? 허참! 어이가 없구만!", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "그리고 교역허가? 허가를 내줄 공무원들이 맨날 숲에서 낮잠이나 자고 있는데, 무슨 수로 허가를 맡느냔 말이지!", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_122_0000
            end
            dq:step(6)
            me:dialog(npc, "에이! 진진처럼 게으르고 굼뜨고 못미더운 사람들 같으니라구!", false, false)
        elseif sel == 1 then
            me:dialog(npc, "준비중입니다.", false, false)
        else
            me:dialog(npc, "그러시군요.", false, false)
        end
        return
    end

    local quest = me:quest(QUEST_TONGTONG)
    local lighthouse = me:quest(QUEST_LIGHTHOUSE)

    if lighthouse == nil or lighthouse:completed() or lighthouse:step() ~= 3 then
        me:dialog(npc, '....', false, false)
        return
    end

    if quest == nil then
        local btn = me:dialog(npc, '안녕하신가, 나는 무기 수집가라네. 자네 대륙 사람 맞지?', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, lb = me:list(npc, '대륙에 망치라는 도구가 있다던데 쓸모가 많다더군. 혹시 하나 구해다 줄 수 있나?', { '네, 구해드리겠습니다.', '글쎄요...' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        local q = me:start_quest(QUEST_TONGTONG)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        me:dialog(npc, '이 일기같은건 잘 보관하고 있을테니, 조심해서 다녀오시게.', false, true)
        return
    end

    if quest:completed() then
        me:dialog(npc, '흠..이 단단하고 묵직한 망치. 정말 쓸 곳이 많구만.', false, true)
        return
    end

    if not me:has_items('망치', 1) then
        me:dialog(npc, '아직 망치라는 것을 구하지 못한것 같군?', false, true)
        return
    end

    local btn = me:dialog(npc, '과연! 묵직하고 단단한 것이 무기로도 손색이 없겠어!', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ['망치'] = 1 } },
        { ['item'] = { ['선장의일기3'] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '아직 망치라는 것을 구하지 못한것 같군?', false, true)
        return
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 선장의일기3을 줄 수 없네.', false, true)
        return
    end
    quest:complete()
    lighthouse:step(4)
    me:push_achievement(32, '진백랑의 부탁을 들어주자. [4/6]', 7, 1)
    me:dialog(npc, '이건 약속했던 누군가의 일기일세! 잘 가시게!', false, true)
end