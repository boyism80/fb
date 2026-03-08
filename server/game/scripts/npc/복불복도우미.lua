-- @note Trash: 2_이벤트\토토.txt "복불복도우미". image omitted. Admin (me:role() >= ROLE.ADMIN): list 몬스터생성/출입구닫기/초기화 -> placeholder. Else: list 귀환 yes/no; yes -> warp_to_return_map (no 노란비서 consume; trash had it commented). sound omitted.

---@brief   NPC 복불복도우미: return warp list; admin (role >= ROLE.ADMIN) gets admin list (placeholders). Uses warp_to_return_map (no item consume).
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
function NPC_340(me, npc)
    if me:role() >= ROLE.ADMIN then
        local sel, btn = me:list(npc, "어떤 명령을 수행하시겠습니까?", {
            "몬스터생성",
            "출입구닫기",
            "초기화",
        }, false)
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    local sel, btn = me:list(npc, "귀환 하시겠습니까?", {
        "네. 나가고 싶어요!",
        "아니요. 아직 안나갈래요!",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        if warp_to_return_map(me) == nil then
            me:dialog(npc, "이동할 수 없습니다.", false, false)
        end
    elseif sel == 1 then
        me:dialog(npc, "잘 생각하셨어요.", false, true)
    end
end
