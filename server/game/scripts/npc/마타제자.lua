-- @note Trash: 1_중국\일반.txt "마타제자". $pamaspiri: 0=accept find 파마의피리, 1=in progress, 4=hand in (has item), 5=done. QUEST_PAMASPIRI: step 1=find, hand-in when has item then complete.

-- @brief   NPC Mata disciple: QUEST_PAMASPIRI — accept to find 파마의피리; when has item, hand in for 마반경 and complete.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (마타제자).
function NPC_273(me, npc)
    local quest = me:quest(QUEST_PAMASPIRI)
    if quest == nil then
        -- Not started: list -> accept
        ::NPC_273_COS001::
        local sel, btn = me:list(npc, "아니, 이게 어디로 갔나... 스승님께 들키기라도 하면 경을 칠 텐데, 큰일이네. 깜빡 졸았던 사이에 누가 빼 갔나? 이를 어쩌지?", { "무슨 잃어버린 물건이라도 있습니까?", "난 가던 길이나 가야지..." }, false)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel == 1 then
            return
        end
        
        ::NPC_273_COS002::
        local button = me:dialog(npc, "예? 음...음... 물건을 하나 잃어버린 것 같은데 통 보이질 않는군요. 겉보기엔 보통 피리인데......", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_273_COS001
        end
        
        ::NPC_273_COS003::
        button = me:dialog(npc, "그 피리는 '파마의피리'라고, 제 스승님께서 귀신을 퇴치하실 때 쓰시는 중요한 물건이지요. 그런데 그게 제가 깜빡 존 사이에......", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_273_COS002
        end
        
        ::NPC_273_COS004::
        sel, btn = me:list(npc, "누가 가져갔는지, 아니면 제가 어디에 떨어뜨렸는지 통 보이질 않네요. 으으... 그 중요한 물건을 잃어버린 것을 스승님께 들키는 날이면...", { "저도 찾는 것을 돕겠습니다.", "그러게 평소에 조심하셨어야죠. 전 이만..." }, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_273_COS003
        end
        if sel == nil or sel == 1 then
            me:dialog(npc, "하긴 제 잘못이겠죠...", false, false)
            return
        end

        quest = me:start_quest(QUEST_PAMASPIRI)
        if quest == nil then
            me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
            return
        end
        if quest then
            quest:step(1)
        end
        me:push_achievement(48, "파마의 피리를 찾자.", 7, 1)
        
        ::NPC_273_COS005::
        button = me:dialog(npc, "예? 정말 그래도 되겠습니까? 이렇게 고마울 데가...", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_273_COS004
        end
        
        ::NPC_273_COS006::
        button = me:dialog(npc, "음, 그럼 이 근처를 먼저 좀 뒤져 주십시오. 사람들이 있으면 피리를 보지 못했느냐고 좀 물어봐도 주시고요. 저는 자리를 비우면 스승님께 들킬까봐 여길 떠날 수가 없네요...", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_273_COS005
        end
        
        ::NPC_273_COS007::
        me:dialog(npc, "아직 스승님께 들키진 않았습니다만 만약 들키기라도 하면... 으으, 불안하군요. 일단 이 부근부터 좀 뒤져 주십시오.", true, false)
        return
    end
    
    if quest:completed() then
        me:dialog(npc, string.format("%s님 덕분에 스승님께 들키지 않고 파마의피리를 원래 자리에 돌려놓을 수 있었습니다. 다신 잃어버리지 않게 조심해야죠.", me:name()), false, false)
        return
    end
    
    local step = quest:step()
    if step == 4 then
        -- step 4 but no 파마의피리 (e.g. lost/sold); completed message is only in quest:completed() above
        if not me:has_items("파마의피리", 1) then
            me:dialog(npc, "파마의피리를 가져오시면 마반경으로 바꿔드리겠네요.", false, false)
            return
        end
        
        -- step 4: has 파마의피리 from 피리부는소년; hand in at 마타제자.
        ::NPC_273_COS0001::
        local button = me:dialog(npc, "앗! 파마의피리를 찾아오셨군요! 고맙습니다. 덕분에 살았습니다.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        
        ::NPC_273_COS0002::
        button = me:dialog(npc, "사례로 '마반경'을 드리죠. 이것은 사악한 힘으로부터 몸을 지켜주는 신기한 물건입니다.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_273_COS0001
        end
        
        local code = me:exchange(
            { ['item'] = { ["파마의피리"] = 1 } },
            { ['item'] = { ["마반경"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "파마의피리가 없어 보이네요.", false, false)
            return
        elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 줄 수 없네요.", false, false)
            return
        end
        
        quest:complete()
        me:push_achievement(48, "파마의 피리를 찾아주었다!", 7, 1)
        
        ::NPC_273_COS0003::
        me:dialog(npc, "어이쿠, 스승님께 들키키 전에 파마의피리를 원래 자리에다 돌려놔야죠. 그럼 전 이만...!", true, false)
        return
    end
    
    me:dialog(npc, "스승님에게 들키기 전에 어서 파마의피리를 찾아주세요.", false, false)
end
