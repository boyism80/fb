-- @note Trash: 2_이벤트\묵언수행.txt "수행간수장". $silence0 (GM-set) → constant 5; email check (fopen/fgets) omitted. 담배 N → warp 부여성 70,140.

local REQUIRED_CIGARETTES = 5

---@brief   NPC 수행간수장: requires 담배 N to release; gives list "준다/주지 않는다", then removes 담배 and warps to 부여성. Email registration check omitted.
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
function NPC_408(me, npc)
    local btn = me:dialog(npc, string.format("음... 담배 [ %d ]개피만 있으면 또 모르겠는데...", REQUIRED_CIGARETTES), false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    if not me:has_items("담배", REQUIRED_CIGARETTES) then
        me:dialog(npc, "자네는 그런 것은 없는 것 같으니... 어쩔 수 없지...", false, true)
        return
    end

    btn = me:dialog(npc, "오오, 그것은 그 귀하다는 담배가 아닌가? 만약 나에게 준다면 내 자네를 내보내 줌세. 어떻게 하겠는가?", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, string.format("그 귀하디 귀한 담배 %d개를 내게 준다면 이곳에서 내보내 주지...", REQUIRED_CIGARETTES), { "준다", "주지 않는다" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 1 then
        me:dialog(npc, "음... 뭐 생각이 바뀌면 언제라도 오게나.", false, false)
        return
    end
    -- sel == 0: "준다"
    if not me:has_items("담배", REQUIRED_CIGARETTES) then
        me:dialog(npc, "음? 그새 담배를 어디다 두었담?", false, false)
        return
    end

    if not me:rmitem("담배", REQUIRED_CIGARETTES, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, "음? 그새 담배를 어디다 두었담?", false, false)
        return
    end

    local exit_map = name2map("부여성")
    if exit_map then
        me:map(exit_map, 70, 140)
    end
end
