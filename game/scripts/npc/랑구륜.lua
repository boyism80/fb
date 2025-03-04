function NPC_2(me, npc)
    local class = me:class()
    if class == 0 then
        npc:dialog(me, '평민은 승급할 수 없습니다.')
        return
    end

    local promotion = me:promotion()
    local name = class2name(class, promotion+1)
    if name == nil then
        npc:dialog(me, '승급할 수 없습니다.')
        return
    end

    npc:list(me, string.format('%s 승급하시겠습니까?', name_with(name, '으로', '로')), {'예', '아니오'})
    me:promotion(promotion+1)

    broadcast(string.format('%s님이 %s 승급하셨습니다. 축하해주세요.', me:name(), name_with(name, '으로', '로')), MESSAGE_TYPE_WORLD, BROADCAST_TYPE_GLOBAL)
    npc:dialog(me, string.format('축하합니다. %s 승급하셨습니다.', name_with(name, '으로', '로')))
end