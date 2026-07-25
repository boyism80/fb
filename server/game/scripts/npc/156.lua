-- npc: 한빙장
local quest = require('lib.quest')
local enum = require('lib.enum')

local ITEM_ICE = '얼음'
local ITEM_ICE_SWORD = '얼음칼'
local ICE_SWORD_COST = 100
local ICE_SWORD_SUCCESS_CHANCE = 8

local AMBER_HELMET_COLORS = {
    { list_text = '황호박보석입니다.', color = '황호박' },
    { list_text = '적호박보석입니다.', color = '적호박' },
    { list_text = '녹호박보석입니다.', color = '녹호박' },
    { list_text = '청호박보석입니다.', color = '청호박' },
    { list_text = '회호박보석입니다.', color = '회호박' },
    { list_text = '갈호박보석입니다.', color = '갈호박' },
    { list_text = '자호박보석입니다.', color = '자호박' },
}

local LIGHT_AMBER_HELMET_COLORS = {
    { list_text = '연녹호박보석입니다', color = '연녹호박' },
    { list_text = '연청호박보석입니다', color = '연청호박' },
    { list_text = '연갈호박보석입니다', color = '연갈호박' },
    { list_text = '연자호박보석입니다', color = '연자호박' },
}

local GOLD_AMBER_HELMET_COLORS = {
    { list_text = '황금호박보석입니다.', color = '황금호박' },
}

local ITEM_IRON_DUST = '쇠가루'
local HELMET_IRON_DUST_COUNT = 2

local function run_ice_sword(me, npc)
    ::NPC_156_0002::
    local sel, btn = me:list(npc, '그래..무슨일로 날 찾아 왔는가??', {
        '얼음칼이 무엇인가요?',
        '얼음칼좀 만들어 주세요..',
        '그냥..인사차 왔어요...잘 계시죠? ^^',
    }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if btn == DIALOG_RESULT.PREV then
        return true
    end
    if sel == nil then
        return false
    end
    if sel == 1 then
        me:dialog(npc, '얼음칼은 얼음으로 만들어진 칼로써 적을 마비시킬 수 있는 능력을 가지고 있지. 단점이 있다면 수리가 안된다고 해야하나? 허허..', { prev = false, next = true })
        return true
    end
    if sel == 2 then
        if not me:has_items(ITEM_ICE, ICE_SWORD_COST) then
            me:dialog(npc, '얼음칼을 만들기 위해선 얼음 100개가 필요하네.', { prev = false, next = false })
            return true
        end
        btn = me:dialog(npc, '오..모두 모아 왔구만.. 수고했네... 어디보자.. 전체적으로 얼음이 싱글탱글 하구먼.. 허허허.. 이보게..이 좋은 얼음이면..장식품으로 써도 좋아 보이는데..정말 얼음칼로 만들고 싶나?.. 흠.. 그래..할 수 없지... 만들어 주겠네...', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return false
        end
        local cost = { ['item'] = { [ITEM_ICE] = ICE_SWORD_COST } }
        local reward = nil
        if math.random(1, 10) <= ICE_SWORD_SUCCESS_CHANCE then
            reward = { ['item'] = { [ITEM_ICE_SWORD] = 1 } }
        end
        local code = me:exchange(cost, reward)
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '얼음칼을 만들기 위해선 얼음 100개가 필요하네.', { prev = false, next = false })
            return true
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 얼음칼을 받을 수 없네.', { prev = false, next = false })
            return true
        end
        if reward == nil then
            me:dialog(npc, '이런..제작 도중에 얼음이 모두 부숴저 버렸군.. 미안하네..', { prev = false, next = false })
        else
            me:dialog(npc, '자..받게나..방금 만든 얼음칼일쎄...어떤가? 감사한가? 껄껄껄..그래..잘 가게..', { prev = false, next = false })
        end
        return true
    end
    if sel == 3 then
        me:dialog(npc, '허허..싱겁기는..', { prev = false, next = true })
        return true
    end
    return true
end

local function run_spirit_summon_shark_weapon(me, npc)
    local q_wr = me:quest(quest.QUEST_WATER_RING)
    if q_wr == nil or not q_wr:completed() then
        return
    end
    local q = me:quest(quest.QUEST_SHARK_WEAPON)
    if q == nil then
        return
    end
    local step = q:step()
    if step == 1 then
        local sel = me:list(npc, ' ', { '혹시 상어장군...' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        sel = me:list(npc, '아 그만 말해도 알겠네..한두명이 말해야 말이지.', { '...' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        sel = me:list(npc, '상어장군 이야기 하는 거지?', { '예 그렇습니다.' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        sel = me:list(npc, '물론 내가 알고 있기는 한데 말이야 그렇게 쉽게 알려 줄수는 없다네..', { '...' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        sel = me:list(npc, '내가 매일같이 쉴 시간도 없이 여기 서 있느냐고 무척이나 몸이 허해져서 말이야.', { '...' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        sel = me:list(npc, '보약이라도 한재 지어먹어야 할 것 같아.', { '!!!' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        sel = me:list(npc, '그래서 말인데 자네가 용왕님께 지어드렸던 보약의 재료를 가지고 오면 내 방법을 알려주도록 하지.', { '헉..별수없군요...', '차라리 모르고 말지..' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        q:step(2)
        me:push_achievement(24, '한빙장의 부탁을 들어주자.', 7, 1)
        if me:dialog(npc, '보약의 재료는 나도 모르니까 자네가 직접 가서 물어보고 지어오라고.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, '아, 그리고 난 산삼도 한뿌리 먹었으면 좋겠으니까 가서 산삼도 한뿌리 가져오시게나.', { prev = false, next = false })
        return
    end

    if step == 2 then
        local items = { '게집게', '게등껍질', '문어다리', '해마꼬리', '산삼' }
        for _, name in ipairs(items) do
            if not me:has_items(name, 1) then
                me:dialog(npc, '보약의 재료를 구하지 못한건가? 용왕님께 지어드렸던 보약의 재료와 산삼 한뿌리 가져오시게나.', { prev = false, next = false })
                return
            end
        end
        for _, name in ipairs(items) do
            if not me:rmitem(name, 1, ITEM_DELETE_TYPE.GIVE) then
                return
            end
        end
        q:step(3)
        me:push_achievement(24, '한빙장의 부탁을 들어주었다.', 7, 1)
        if me:dialog(npc, '오. 재료를 다 모아왔군.\n\n그럼 잠시 기다리게나. 내 이걸 먼저 보약으로 만들어 먹고..', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        if me:dialog(npc, '.\n\n..\n\n...', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        if me:dialog(npc, '캬. 몸에 기운이 솟는구만. 솟아.\n\n그럼 받을대로 다 받았으니 슬슬 말해 주기로 하지.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        if me:dialog(npc, '사실 그 제작방법은 상어장군만이 알고 있다고 하더군.\n\n하지만 상어장군이 죽은 이 마당에...', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, '따라서 그를 저승에서 불러내서 물어봐야 한다네..\n\n그를 저승에서 불러내는 방법은 제단을 담당하고 있는 사람한테 가서 직접 물어보시게나.', { prev = false, next = false })
        return
    end

    if step >= 3 then
        me:dialog(npc, '....', { prev = false, next = false })
    end
end

local function run_amber_helmet_craft(me, npc, colors, has_prev)
    local options = {}
    for i = 1, #colors do
        options[i] = colors[i].list_text
    end
    local sel, btn = me:list(npc, '그럼.. 어떤 색깔의 호박보석을 가지고 왔는가?', options, { prev = has_prev })
    if btn == DIALOG_RESULT.QUIT then
        return DIALOG_RESULT.QUIT
    end
    if btn == DIALOG_RESULT.PREV then
        return DIALOG_RESULT.PREV
    end
    if sel == nil or sel < 1 or sel > #colors then
        return DIALOG_RESULT.QUIT
    end
    local color_name = colors[sel].color
    local gem_name = color_name .. '보석'
    local helmet_name = color_name .. '투구'
    local confirm_opt, confirm_btn = me:list(npc, '그래.. ' .. gem_name .. '으로 어떤 아이템을 만들텐가?', { color_name .. '투구입니다.' }, { prev = false })
    if confirm_btn == DIALOG_RESULT.QUIT then
        return DIALOG_RESULT.QUIT
    end
    if confirm_opt ~= 1 then
        return DIALOG_RESULT.NEXT
    end
    local code = me:exchange(
        { ['item'] = { [gem_name] = 1, [ITEM_IRON_DUST] = HELMET_IRON_DUST_COUNT } },
        { ['item'] = { [helmet_name] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        return me:dialog(npc, '자네 혹시 재료를 모르는 것인가? ' .. gem_name .. '과 쇠가루 2개를 가지고 와야 ' .. helmet_name .. '를 만들어 줄 수 있네.', { prev = false, next = true })
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        return me:dialog(npc, '소지품이 가득 차서 ' .. helmet_name .. '를 받을 수 없네.', { prev = false, next = true })
    end
    return me:dialog(npc, gem_name .. '으로 ' .. helmet_name .. '를 만들어주었네.', { prev = false, next = true })
end

return {
    on_click = function(me, npc)
        ::NPC_156_0001::
        local sel, btn = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {
            '얼음칼만들기',
            '초혼술방법',
            '호박투구만들기',
            '연호박투구만들기',
            '황금호박투구만들기',
        }, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end
        if sel == 1 then
            if run_ice_sword(me, npc) == false then
                return
            end
        elseif sel == 2 then
            run_spirit_summon_shark_weapon(me, npc)
        elseif sel == 3 then
            local r = run_amber_helmet_craft(me, npc, AMBER_HELMET_COLORS, true)
            if r == DIALOG_RESULT.QUIT then
                return
            end
            if r == DIALOG_RESULT.PREV then
                goto NPC_156_0001
            end
        elseif sel == 4 then
            local r = run_amber_helmet_craft(me, npc, LIGHT_AMBER_HELMET_COLORS, true)
            if r == DIALOG_RESULT.QUIT then
                return
            end
            if r == DIALOG_RESULT.PREV then
                goto NPC_156_0001
            end
        elseif sel == 5 then
            local r = run_amber_helmet_craft(me, npc, GOLD_AMBER_HELMET_COLORS, true)
            if r == DIALOG_RESULT.QUIT then
                return
            end
            if r == DIALOG_RESULT.PREV then
                goto NPC_156_0001
            end
        end
        goto NPC_156_0001
    end
}
