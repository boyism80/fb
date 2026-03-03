-- @note Trash: 1_세시마을\유두집.txt "백리향". Active when gv("sesi_rightnow") == 8 (유두). List: request songpyeon ingredients (task), or hand in "weeding" for random reward. $sesicount = quest:progress() (incremented elsewhere when player weeds).

-- @brief   Handles "송편재료가 필요해요.": dialogs, starts quest and sets step(1), progress(0).
-- @param[in]  me    The character talking to the NPC.
-- @param[in]  npc   The NPC entity (백리향).
-- @param[in]  quest Current QUEST_BAEKRIHYANG quest or nil.
local function run_songpyeon_request(me, npc, quest)
    local btn = me:dialog(npc, "송편재료가 필요하시다구요? 아... 세인이 언니에게 얘기듣고 오셨나보군요?", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "음.. 제가 지금 벌초를 해야하는데.. 대신 해주시면 송편재료를 드릴께요.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "저희마을 서쪽에 가면 묘지가 있는데.. 무덤주위에 잡초들이 자라 있거든요.. 잡초들을 좀 없애주세요..", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "그럼 부탁드릴께요~~~", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if quest == nil then
        quest = me:start_quest(QUEST_BAEKRIHYANG)
        if quest == nil then
            return
        end
    end
    if quest and not quest:completed() then
        quest:step(1)
        quest:progress(0)
    end
end

-- @brief   Handles "벌초를 하고 왔어요.": checks step/progress/completed, then gives random songpyeon ingredients and completes quest.
-- @param[in]  me    The character talking to the NPC.
-- @param[in]  npc   The NPC entity (백리향).
-- @param[in]  quest Current QUEST_BAEKRIHYANG quest (must not be nil when called for this selection).
local function run_weeding_handin(me, npc, quest)
    if quest == nil or quest:step() < 1 then
        me:dialog(npc, "벌초를 하지 않으셨네요? 절 속이시면 안돼죠..", false, false)
        return
    end
    if quest:progress() < 1 then
        me:dialog(npc, "벌초를 하지 않으셨네요? 절 속이시면 안돼죠..", false, false)
        return
    end
    if quest:completed() then
        me:dialog(npc, "이미 송편재료를 받으셨죠. 세인이 언니에게 가서 맛있는 송편 만들어달라고 하세요. ^^", false, false)
        return
    end

    local btn = me:dialog(npc, "어머.. 벌초를 하고 오셨나요?", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "아.. 감사합니다! 그럼 약속 드린대로 재료를 드릴께요..", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "그럼 세인이언니에게 가서 맛있는 송편 만들어달라고 하세요. ^^", false, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local r = math.random(1, 10)
    local items
    if r <= 4 then
        items = { ["호두"] = 1, ["깨"] = 1, ["쌀가루"] = 1 }
    elseif r <= 8 then
        items = { ["밤"] = 1, ["깨"] = 1, ["쌀가루"] = 1 }
    else
        items = { ["식용호박"] = 1, ["깨"] = 1, ["쌀가루"] = 1 }
    end
    if me:mkitem(items) == nil then
        me:dialog(npc, "소지품이 가득 차서 재료를 줄 수 없습니다.", false, false)
        return
    end
    quest:progress(0)
    quest:complete()
end

-- @brief   NPC Baekrihyang: seasonal (sesi_rightnow == 8) list; gives weeding task (step 1, progress 0) or rewards when quest:progress() >= 1.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (백리향).
function NPC_363(me, npc)
    local btn = me:dialog(npc, "안녕하세요? 저는 백리향입니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    if gv("sesi_rightnow") ~= 8 then
        return
    end

    local quest = me:quest(QUEST_BAEKRIHYANG)
    local sel, list_btn = me:list(npc, "무슨일로 절 찾아오셨나요?", {
        "송편재료가 필요해요.",
        "벌초를 하고 왔어요."
    }, false)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 0 then
        run_songpyeon_request(me, npc, quest)
        return
    end

    run_weeding_handin(me, npc, quest)
end
