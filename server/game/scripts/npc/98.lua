-- npc: 용노인

local OPT_CAVE = '용왕굴이 어디 인가요?'
local OPT_HELLO = '그냥 인사하러 왔어요...!!'

local REQUIRED_FANGS = {
    '진룡의어금니',
    '묵룡의어금니',
    '흑룡의어금니',
    '감룡의어금니'
}

local DESTINATIONS = {
    {
        option = '용왕용마가 사는 곳으로...',
        map = '용왕용마의침소',
        mob = '용왕용마',
        confirm = '좋아.. 그럼 용왕용마가 사는 곳으로 데려다 주지..'
    },
    {
        option = '용왕용천이 사는 곳으로...',
        map = '용왕용천의침소',
        mob = '용왕용천',
        confirm = '좋아.. 그럼 용왕용천이 사는 곳으로 데려다 주지..'
    },
    {
        option = '용왕용랑이 사는 곳으로...',
        map = '용왕용랑의침소',
        mob = '용왕용랑',
        confirm = '좋아.. 그럼 용왕용랑이 사는 곳으로 데려다 주지..'
    },
    {
        option = '용왕용겸이 사는 곳으로...',
        map = '용왕용겸의침소',
        mob = '용왕용겸',
        confirm = '좋아.. 그럼 용왕용겸이 사는 곳으로 데려다 주지..'
    }
}

local INVENTORY_CAPACITY = 52

local function find_nearby_character(me, name)
    local map = me:map()
    if map == nil then
        return nil
    end

    local nears = map:nears({me:position()}, OBJECT_TYPE.CHARACTER)
    for _, ch in pairs(nears) do
        if ch:name() == name then
            return ch
        end
    end

    return nil
end

local function has_empty_slot(ch)
    local count = 0
    for _ in pairs(ch:items()) do
        count = count + 1
    end
    return count < INVENTORY_CAPACITY
end

return {
    on_click = function(me, npc)
    end,

    on_hello = function(me, npc)
        local OPT_FANG_YES = '네.. 여기 4가지의 어금니를 모두 가져왔습니다.'
        local OPT_FANG_NO = '아니오.. 그게 무엇인가요?'
        local OPT_FRIEND_YES = '네.. 같이 갈 친구가 있어요..'
        local OPT_FRIEND_NO = '아니오.. 혼자 갈래요..'

        local dest_options = {}
        for i, dest in ipairs(DESTINATIONS) do
            dest_options[i] = dest.option
        end

        ::NPC_98_0001::
        local selected, button = me:pursuit(npc, '안녕하신가? 여긴 무슨일로 찾아 오셨는가?', { OPT_CAVE, OPT_HELLO })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if selected ~= OPT_CAVE then
            return
        end

        ::NPC_98_0002::
        button = me:dialog(npc, '글쎄.. 용왕굴에 입장하기란 여간 힘들지 않다네..', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_98_0003::
        button = me:dialog(npc, '4대 수호신 즉, 흑룡,진룡,묵룡,감룡의 기운이 길을 막고 있어서 말일세..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0002
        end

        ::NPC_98_0004::
        button = me:dialog(npc, '하지만, 만약 4대수호신의 어금니만 있다면 불가능한 일도 아니지..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0003
        end

        selected, button = me:list(npc, '혹시.. 어금니를 가져 오셨는가?', { OPT_FANG_YES, OPT_FANG_NO })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if selected ~= 1 then
            return
        end

        ::NPC_98_0006::
        button = me:dialog(npc, '그런가?', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_98_0007::
        button = me:dialog(npc, '정말 가져 왔는지 확인하는데 앞서 먼저 일러둘 말이 있는데..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0006
        end

        ::NPC_98_0008::
        button = me:dialog(npc, '어금니를 모두 다 주고난뒤\n끝까지 내 말을 잘 들어야 하네..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0007
        end

        ::NPC_98_0009::
        button = me:dialog(npc, '그렇지 않고 일어나는 일에\n대해서는 내가 책임을 질 수가\n없다네..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0008
        end

        ::NPC_98_0010::
        button = me:dialog(npc, '그리고 한번에 2명까지 용왕이\n사는 곳으로 보내 줄 수 있으니\n같이 갈 친구가 있으면 지금\n데려오게..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0009
        end

        ::NPC_98_0011::
        button = me:dialog(npc, '한가지 일러두자면.. 각\n용왕방에는 알 수 없는 기운이\n항시 감돌고 있다는 것일세..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0010
        end

        ::NPC_98_0012::
        button = me:dialog(npc, '그 기운은 평상시에는 알 수 없으나, 자네가 만약 각 용왕방의 용왕이 쓰러진다면.. 글쎄...', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0011
        end

        ::NPC_98_0013::
        button = me:dialog(npc, '아마도, 그 기운으로 인하여\n용왕방에 있는 모든 인원이\n용왕방에서 강제로 나가게 될 수\n있다는 것일세...', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0012
        end

        ::NPC_98_0014::
        button = me:dialog(npc, '한 가지 더 일러두자면, 용왕이\n쓰러질 때 가방의 빈 자리로\n아이템을 즉시 획득하게 된다고\n하네.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0013
        end

        ::NPC_98_0015::
        button = me:dialog(npc, '아마도..가방에 빈 자리가 없다면\n아이템을 획득하지 못할테니.. 꼭\n명심하게나', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0014
        end

        ::NPC_98_0016::
        button = me:dialog(npc, '운이 정말 좋다면.. 용무기의 재료인 용왕의발톱의 주인이 될 수도 있겠지만..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0015
        end

        ::NPC_98_0017::
        button = me:dialog(npc, '하늘의 선택을 받은 자만이 얻을 수 있다고 하니.. 이 점을 항시 감안하고 있게나..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0016
        end

        ::NPC_98_0018::
        button = me:dialog(npc, '자, 그럼 어디 보세나.. 정말 4가지의 어금니를 가져 왔는지..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0017
        end

        if not me:has_items({
            ['진룡의어금니'] = 1,
            ['묵룡의어금니'] = 1,
            ['흑룡의어금니'] = 1,
            ['감룡의어금니'] = 1
        }) then
            me:dialog(npc, '아직 어금니가 준비되지 않았는데?', { prev = false, next = true })
            return
        end

        ::NPC_98_0019::
        button = me:dialog(npc, '좋아.. 이걸로 내가 용왕이 사는 곳으로 데려다 주겠네..', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_98_0020::
        button = me:dialog(npc, '용왕은 모두 4마리가 있다네..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0019
        end

        ::NPC_98_0021::
        button = me:dialog(npc, '전사의 기운을 가지고 있는\n용왕용마..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0020
        end

        ::NPC_98_0022::
        button = me:dialog(npc, '도적의 기운을 가지고 있는\n용왕용천..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0021
        end

        ::NPC_98_0023::
        button = me:dialog(npc, '주술사의 기운을 가지고 있는\n용왕용랑..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0022
        end

        ::NPC_98_0024::
        button = me:dialog(npc, '도사의 기운을 가지고 있는\n용왕용겸.. 이 그것이네..', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_98_0023
        end

        selected, button = me:list(npc, '자네는 어디로 가길 원하나?', dest_options)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if selected == nil or selected < 1 or selected > #DESTINATIONS then
            return
        end

        local dest = DESTINATIONS[selected]
        local map = name2map(dest.map)
        if map == nil then
            me:dialog(npc, '존재하지 않는 맵입니다.', { prev = false, next = true })
            return
        end

        button = me:dialog(npc, dest.confirm, { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        selected, button = me:list(npc, '혹시 같이 갈 친구가 있나?', { OPT_FRIEND_YES, OPT_FRIEND_NO })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local friend = nil
        if selected == 1 then
            ::NPC_98_0029::
            local friend_name = me:input(npc, '함께 갈 친구의 이름은 무엇인가?', { top = '예. 제 친구의 이름은', bottom = '입니다.', maxlen = 12, prev = false })
            if friend_name == DIALOG_RESULT.QUIT or type(friend_name) ~= 'string' or friend_name == '' then
                return
            end
            if friend_name == me:name() then
                button = me:dialog(npc, '자신과는 함께 갈 수 없다네.', { prev = false, next = true })
                if button == DIALOG_RESULT.QUIT then
                    return
                end
                goto NPC_98_0029
            end

            local found = find_nearby_character(me, friend_name)
            if found == nil then
                button = me:dialog(npc, name_with(friend_name, '은', '는') .. ' 근처에 없습니다.', { prev = false, next = true })
                if button == DIALOG_RESULT.QUIT then
                    return
                end
                goto NPC_98_0029
            end

            friend = found

            button = me:dialog(npc, '먼저, 동료의 가방에 빈 자리가 있는지 확인하고 보내주겠네.', { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if not has_empty_slot(friend) then
                me:dialog(npc, '동료의 가방에 빈 자리가 없다네.', { prev = false, next = true })
                return
            end
        end

        button = me:dialog(npc, '그럼.. 행운을 빌겠네..', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if not me:has_items({
            ['진룡의어금니'] = 1,
            ['묵룡의어금니'] = 1,
            ['흑룡의어금니'] = 1,
            ['감룡의어금니'] = 1
        }) then
            me:dialog(npc, '아직 어금니가 준비되지 않았는데?', { prev = false, next = true })
            return
        end

        local inst = map:clone()
        if inst == nil then
            me:dialog(npc, '용왕굴을 열 수 없다네.', { prev = false, next = true })
            return
        end

        local dirs = { DIRECTION.TOP, DIRECTION.RIGHT, DIRECTION.BOTTOM, DIRECTION.LEFT }
        if inst:spawn_mob(dest.mob, 10, 10, dirs[math.random(1, 4)]) == nil then
            inst:destroy()
            me:dialog(npc, '용왕굴을 열 수 없다네.', { prev = false, next = true })
            return
        end

        for _, name in ipairs(REQUIRED_FANGS) do
            me:rmitem(name, 1, ITEM_DELETE_TYPE.GIVE)
        end

        if me:map(inst, { math.random(11, 14), math.random(22, 24) }) ~= true then
            inst:destroy()
            return
        end
        if friend ~= nil then
            friend:map(inst, { math.random(11, 14), math.random(22, 24) })
        end
    end
}
