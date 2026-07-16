-- npc: 방송
function NPC_73(me, npc)
    me:dialog(npc, "전 서버에 방송을 하시려구요?", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return;
    end

    ::NPC_73_1::
    if not me:has_items("방송쿠폰", 1) then
        me:dialog(npc, "방송쿠폰을 가지고 계셔야 방송을 하실 수 있어요.", { prev = false, next = true })
        return;
    end

    local message = me:input(npc, "방송하실 말을 적어주세요.", { top = "방송할 말은", bottom = "입니다", maxlen = 35, prev = true })
    if message == DIALOG_RESULT.QUIT then
        return;
    end

    if message == DIALOG_RESULT.PREV then
        goto NPC_73_1
    end

    me:rmitem("방송쿠폰", 1, ITEM_DELETE_TYPE.GIVE)
    broadcast(string.format("###[%s]>>%s", me:name(), message), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.GLOBAL)
end
