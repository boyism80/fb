function NPC_98(me, npc)
    ::NPC_98_0001::
    local button = me:dialog(npc, '용왕의침소로 이동하기 위해서는, 어금니 4개가 필요하네.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_98_0002::
    button = me:dialog(npc, '어금니의 이름들은 각각 알고 왔겠지?', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_98_0001
    end

    ::NPC_98_0003::
    local selected, button = me:list(npc, '지금 준비되어 있나?\n참고로 그룹원이 있으면 같이 이동된다네.', {'네', '아니요'}, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_98_0002
    end

    if selected == 1 then
        return
    end

    local map_names = {'용왕용마의침소', '용왕용천의침소', '용왕용랑의침소', '용왕용겸의침소'}
    selected, button = me:list(npc, '어디로 이동하겠는가?', map_names, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_98_0003
    end

    local map = name2map(map_names[selected+1])
    if map == nil then
        me:dialog(npc, '존재하지 않는 맵입니다.', false, true)
        return
    end

    local required_items = {
        '묵룡의어금니',
        '감룡의어금니',
        '흑룡의어금니',
        '진룡의어금니'
    }
    local required_table = {}
    for _, name in ipairs(required_items) do
        required_table[name] = 1
    end

    if not me:has_items(required_table) then
        me:dialog(npc, '아직 어금니가 준비되지 않았는데?', false, true)
        return
    end

    for _, required_item in ipairs(required_items) do
        me:rmitem(required_item, 1, ITEM_DELETE_TYPE.GIVE)
    end

    local group = me:group()
    local members = {}
    if group == nil then
        table.insert(members, me)
    else
        for _, member in pairs(group:members()) do
            table.insert(members, member)
        end
    end

    for _, member in pairs(members) do
        local ch = name2ch(member)
        if ch ~= nil then
            local x = math.random(11, 14)
            local y = math.random(22, 24)
            ch:map(map, {x, y})
        end
    end
end