-- @note Trash: 1_환상의섬\일반.txt "유령화화". At specific hours (1,4,7,10,13,16,19,22) and minute 10~40, trash replaces NPC with "화화" (npcspawn/deletenpc) and shows image. Not implemented: npcspawn, deletenpc, image. Dialog flow only.

---@brief NPC 유령화화: phantom flower; dialog about a distant girl on the cliff.
---@param[in] me  The character.
---@param[in] npc The NPC entity.
function NPC_380(me, npc)
::NPC_380_0000::
    local btn = me:dialog(npc, "멀리 어렴풋한 무언가가 보인다.\n\n절벽에 가까이 서서 보면 멀리 소녀가 서있는 것 같다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "먼곳을 바라보고 있는 소녀의 얼굴은 어쩐지 슬퍼보인다.\n\n불러보고, 손짓을 해도 소녀는 아무런 대답이 없다.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_380_0000
    end
end
