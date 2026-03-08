-- @brief   NPC Baek Nayeon: gives 깨진김장독 during Chunghwa (QUEST_BAEK_MONGYEON) cleanup quest.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity.
-- @note    Only when gv("sesi_rightnow") == 2 and quest step >= 1; gives once (player must not already have item).
function NPC_354(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 백나연입니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if gv("sesi_rightnow") ~= 2 then
        return
    end

    local quest = me:quest(QUEST_BAEK_MONGYEON)
    if quest == nil then
        return
    end

    if quest:step() < 1 then
        return
    end

    if me:has_items("깨진김장독", 1) then
        return
    end

    button = me:dialog(npc, "몽연오빠가 청소한다구요? 잘됐다~~ 김장독 깨진게 있어서 어떻게할지 고민하고 있었는데.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "깨진거니까 다치지 않도록 조심해서 가져가세요! 감사합니다!!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("깨진김장독", 1)
end
