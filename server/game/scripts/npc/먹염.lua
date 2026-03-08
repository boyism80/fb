function NPC_168(me, npc)
    local food_rewards = {
        ['바나나다발'] = { item = nil,  item_count = 0,  money = 500 },
        ['영양반찬'] = { item = nil,  item_count = 0,  money = 5000 },
        ['영양음료'] = { item = nil,  item_count = 0,  money = 5000 },
        ['영양통구이'] = { item = nil,  item_count = 0,  money = 0 },
        ['초보도시락'] = { item = '북천황변신시약',  item_count = 1,  money = 0 },
        ['기분전환반찬'] = { item = nil,  item_count = 0,  money = 0 },
        ['기분전환음료'] = { item = nil,  item_count = 0,  money = 30000 },
        ['기분전환통구이'] = { item = nil,  item_count = 0,  money = 30000 },
        ['활력충전통구이'] = { item = nil,  item_count = 0,  money = 50000 },
        ['활력충전반찬'] = { item = nil,  item_count = 0,  money = 0 },
        ['활력충전음료'] = { item = nil,  item_count = 0,  money = 50000 },
        ['궁극의도시락'] = { item = '양첨목봉',  item_count = 1,  money = 0 },
        ['생장촉진통구이'] = { item = '청의태자변신시약',  item_count = 1,  money = 0 },
        ['생장촉진반찬'] = { item = nil,  item_count = 0,  money = 0 },
        ['생장촉진음료'] = { item = nil,  item_count = 0,  money = 0 },
        ['전설의도시락'] = { item = '흑형도',  item_count = 1,  money = 0 },
        ['생명보존통구이'] = { item = '토깽이변신시약',  item_count = 2,  money = 0 },
        ['생명보존반찬'] = { item = nil,  item_count = 0,  money = 0 },
        ['생명보존음료'] = { item = nil,  item_count = 0,  money = 0 },
        ['환상의도시락'] = { item = '천풍선',  item_count = 1,  money = 0 },
        ['1차도시락'] = { item = '암흑왕변신시약',  item_count = 1,  money = 0 },
        ['2차도시락'] = { item = '진명삼지창',  item_count = 1,  money = 0 },
        ['3차도시락'] = { item = '칠교칠선',  item_count = 1,  money = 0 },
        ['4차도시락'] = { item = '금마반지',  item_count = 2,  money = 0 },
        ['비장의도시락'] = { item = '이가닌자의검',  item_count = 1,  money = 0 },
    }
    
    local quest = me:quest(QUEST_MEOKYEOM)
    local btn
    
    if quest == nil then
        ::NPC_168_COS001::
        btn = me:dialog(npc, '곧 우리 아버님이 환갑이 되시지. 아~ 이 못난 아들을 기르시느라 정말 고생 많이 하셨지. 이젠 내가 호강시켜 드려야 할 텐데... 오래오래 사셨으면 좋겠어.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_168_COS002::
        btn = me:dialog(npc, '환갑잔치에 쓸 요리가 필요한데, 변변한 요리사가 없어서 말이야. 환갑잔치는 평생 한번 있는 경사인데, 그런 날에 맛없는 요리를 드셔서야 안 되지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_168_COS001
        end
        local sel, lb = me:list(npc, '아버님께 맛있는 것을 많이 드시게 하고 싶은데, 요리사를 구할 수가 없으니 참 큰일이야..', { '저도 요리를 만들 줄 아는데, 도와드릴까요?', '찬찬에게 맡겨보시지 그러세요?' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel == 1 then
            if sel == 1 then
                me:dialog(npc, '그렇다면 좋겠지만 찬찬은 너무 멀리 있어서 말이지...', false, false)
            end
            return
        end
        if sel ~= 0 then
            return
        end
        if not me:start_quest(QUEST_MEOKYEOM) then
            return
        end
        quest = me:quest(QUEST_MEOKYEOM)
        quest:step(1)
        ::NPC_168_COS004::
        btn = me:dialog(npc, '자네가 도와주겠다고? 허허! 그거 듣던 중 반가운 소리구만! 실은 자네에게 이런 이야기를 한 것도 자네를 유심히 보니 요리 깨나 해 본 사람 같아서였지. 허허허..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_168_COS005::
        btn = me:dialog(npc, '그래, 어떤 요리든 좋네. 맛있는 요리라면 뭐든지 좋아. 요리를 가져오면 요리가 얼마나 맛있는가에 따라 사례를 하도록 하겠네.\n\n자자, 이러고 있지 말고 서둘러 주게. 난 여기서 기다리고 있겠네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_168_COS004
        end
        return
    end

    if quest:step() == 0 then
        ::NPC_168_COS001B::
        btn = me:dialog(npc, '곧 우리 아버님이 환갑이 되시지. 아~ 이 못난 아들을 기르시느라 정말 고생 많이 하셨지. 이젠 내가 호강시켜 드려야 할 텐데... 오래오래 사셨으면 좋겠어.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_168_COS002B::
        btn = me:dialog(npc, '환갑잔치에 쓸 요리가 필요한데, 변변한 요리사가 없어서 말이야. 환갑잔치는 평생 한번 있는 경사인데, 그런 날에 맛없는 요리를 드셔서야 안 되지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_168_COS001B
        end
        local sel, lb = me:list(npc, '아버님께 맛있는 것을 많이 드시게 하고 싶은데, 요리사를 구할 수가 없으니 참 큰일이야..', { '저도 요리를 만들 줄 아는데, 도와드릴까요?', '찬찬에게 맡겨보시지 그러세요?' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel == 1 then
            if sel == 1 then
                me:dialog(npc, '그렇다면 좋겠지만 찬찬은 너무 멀리 있어서 말이지...', false, false)
            end
            return
        end
        if sel ~= 0 then
            return
        end
        if not me:start_quest(QUEST_MEOKYEOM) then
            return
        end
        quest = me:quest(QUEST_MEOKYEOM)
        quest:step(1)
        ::NPC_168_COS004B::
        btn = me:dialog(npc, '자네가 도와주겠다고? 허허! 그거 듣던 중 반가운 소리구만! 실은 자네에게 이런 이야기를 한 것도 자네를 유심히 보니 요리 깨나 해 본 사람 같아서였지. 허허허..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_168_COS005B::
        btn = me:dialog(npc, '그래, 어떤 요리든 좋네. 맛있는 요리라면 뭐든지 좋아. 요리를 가져오면 요리가 얼마나 맛있는가에 따라 사례를 하도록 하겠네.\n\n자자, 이러고 있지 말고 서둘러 주게. 난 여기서 기다리고 있겠네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_168_COS004B
        end
        return
    end
    
    ::NPC_168_HAND_IN::
    local sel = me:list(npc, '오, 기다리고 있었네, 요리는 만들어 왔는가?', { '예, 만들어 왔습니다.', '아니오, 아직..' })
    if sel == nil then
        return
    end
    if sel == 1 then
        me:dialog(npc, '흠.. 곧 환갑잔치가 시작하니 조금만 서둘러 주게.', false, false)
        return
    end
    if sel ~= 0 then
        return
    end

    ::NPC_168_COS006::
    btn = me:dialog(npc, '그런가! 수고했네. 그럼 어떤 요리를 가져왔는지 꺼내 보게나.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_168_HAND_IN
    end
    local food_name = me:input(npc, '요리 이름인 뭔가?', '제가 만든 요리는', '입니다.', 15, true)
    if food_name == nil or food_name == DIALOG_RESULT.QUIT then
        return
    end
    if food_name == DIALOG_RESULT.PREV then
        goto NPC_168_COS006
    end
    if type(food_name) ~= 'string' or food_name == '' then
        return
    end
    
    local row = food_rewards[food_name]
    if row == nil then
        me:dialog(npc, food_name .. '!! 그거 좋지... 하지만 자네는 그 요리를 가지고 있지 않은데? 어디 놔 두고 온 것은 아닌가? 쯧쯧...', false, false)
        return
    end
    
    local has_reward_item = (row.item ~= nil and row.item_count > 0)
    local has_reward_money = (row.money ~= nil and row.money > 0)

    if not has_reward_item and not has_reward_money then
        if not me:has_items(food_name, 1) or not me:rmitem(food_name, 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, food_name .. '!! 그거 좋지... 하지만 자네는 그 요리를 가지고 있지 않은데? 어디 놔 두고 온 것은 아닌가? 쯧쯧...', false, false)
            return
        end
        me:dialog(npc, '정말 고맙네! 수고했네. 앞으로도 계속 부탁하네.', true, true)
        return
    end

    local reward = {}
    if has_reward_item then
        reward['item'] = { [row.item] = row.item_count }
    end
    if has_reward_money then
        reward['money'] = row.money
    end

    local code = me:exchange(
        { ['item'] = { [food_name] = 1 } },
        reward
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, food_name .. '!! 그거 좋지... 하지만 자네는 그 요리를 가지고 있지 않은데? 어디 놔 두고 온 것은 아닌가? 쯧쯧...', false, false)
        return
    elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 보상을 줄 수 없네.', false, true)
        return
    end
    me:dialog(npc, '정말 고맙네! 수고했네. 앞으로도 계속 부탁하네.', true, true)
end