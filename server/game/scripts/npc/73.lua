-- npc: 방송

return {
    on_click = function(me, npc)
        local btn = me:dialog(npc, "전 서버에 방송을 하시려구요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_73_0001::
        if not me:has_items("방송쿠폰", 1) then
            local btn = me:dialog(npc, "방송쿠폰을 가지고 계셔야 방송을 하실 수 있어요.", { prev = false, next = false })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return;
        end

        local message = me:input(npc, "방송하실 말을 적어주세요.", { top = "방송할 말은", bottom = "입니다", maxlen = 35, prev = true })
        if message == DIALOG_RESULT.QUIT then
            return;
        end

        if message == DIALOG_RESULT.PREV then
            goto NPC_73_0001
        end

        if type(message) ~= 'string' or message == '' or #CP949(message) > 35 then
            me:dialog(npc, "방송할 말이 비었거나 너무 깁니다.", { prev = false, next = false })
            return
        end

        if not me:rmitem("방송쿠폰", 1, ITEM_DELETE_TYPE.GIVE) then
            return
        end
        me:shout(message, "###[{}]>>{}", MESSAGE_TYPE.WORLD, BROADCAST_TYPE.GLOBAL, { log = 'broadcast_coupon' })
    end
}
