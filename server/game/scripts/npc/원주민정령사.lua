function NPC_113(me, npc)
    local dq = me:quest(QUEST_DETECTIVE)
    if dq and not dq:completed() and dq:step() == 12 then
        local sel, list_btn = me:list(npc, "수사관이 왔는가? 올바른 기운이 느껴지는도다.", {
            "특별한 옷에 대해 듣고자 합니다.",
            "문화재 유출에 대해 듣고자 합니다.",
            "그냥 지나가던 길입니다.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            -- fall through to TOTEM_CLOTHES logic below
        elseif sel == 1 then
            local btn = me:dialog(npc, "정령들이 화를 내고 있다. 아, 정령들이 집으로 삼는 토템들이 이 섬을 떠나는구나.", false, true)
            if btn == DIALOG_RESULT.QUIT then return end
            dq:step(13)
            me:dialog(npc, "보름달이 뜨면 정령들이 분노를 이기지 못하고 비명을 내지른다. 아아, 이를 어이해야 좋단 말이더냐.", true, false)
            return
        else
            me:dialog(npc, ".............", false, false)
            return
        end
    end

    local totem_names = { '번개의토템', '바람의토템', '대지의토템', '화염의토템' }
    local armor_names = {
        '황혼의갑주', '여명의연갑', '황혼의활복', '여명의도복',
        '황혼의도포', '여명의치마', '황혼의장삼', '여명의통옷',
    }
    local quest = me:quest(QUEST_TOTEM_CLOTHES)
    local btn

    local function count_slots(items)
        local n = 0
        for _ in pairs(items or {}) do
            n = n + 1
        end
        return n
    end

    if quest == nil then
        btn = me:dialog(npc, '황혼과 여명은 각각 해가 지고 뜨는 때...\n정령의 활동이 가장 활성화되는 때...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, lb = me:list(npc, '나는 이 활성화 된 정령의 힘을 모두 결집한, 황혼의 옷과 여명의 옷을 만드는 자...', { '제게도 옷을 만들어주세요.', '수고하세요.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_113_COS001::
        btn = me:dialog(npc, '그러나 옷을 만드는 데에는 재료가 필요하다...\n정령의 힘을 담은 재료가 필요하다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_113_COS002::
        btn = me:dialog(npc, '외부인인 그대가 과연 정령의 힘을 이해하고 그 재료가 어떤 것인지 깨달을 수 있을까...\n깨닫는 것은 그대가 할 일... 재료를 구해오는 것도 그대가 할 일... 나는 재료로 옷을 만들 뿐...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS001
        end

        local has_any_totem = false
        for _, name in ipairs(totem_names) do
            if count_slots(me:items(name)) > 0 then
                has_any_totem = true
                break
            end
        end
        if not has_any_totem then
            me:dialog(npc, '외부인이여... 그대는 아직 깨닫지 못했다... 그대는 아직 정령의 힘을 담은 재료가 무엇인지 모르고 있다...', true, true)
            return
        end

        ::NPC_113_COS003::
        btn = me:dialog(npc, '그렇다... 그대는 정령의 힘을 깨달았다... 토템이야말로 정령의 힘을 담은 재료...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_113_COS004::
        btn = me:dialog(npc, '화염, 번개, 대지, 바람의토템... 모두 4가지의 토템이 3개씩 모여야 한다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS003
        end
        ::NPC_113_COS005::
        btn = me:dialog(npc, '그대는 비록 외부인이지만 정령의 힘을 이해했다... 노력에 대한 보답으로... 다음 재료가 무엇인지 가르쳐준다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS004
        end
        ::NPC_113_COS006::
        btn = me:dialog(npc, '황혼과 여명 사이에는... 정령들의 활동의 증거로 \'이슬\'이 맺힌다...\n그 이슬을 50방울 모아 마음으로부터 기원하면... 어떤 특별한 이슬... \'여신의이슬\'을 만들 수 있다...\n그 이슬 또한 옷의 재료다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS005
        end
        btn = me:dialog(npc, '폭염도의 북쪽으로 가라... 이슬을 얻을 수 있을 것이다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS006
        end

        quest = me:start_quest(QUEST_TOTEM_CLOTHES)
        if quest == nil then
            return
        end
        quest:step(1)
        me:push_achievement(42, '투명한 이슬을 구하자.', 7, 16)
        return
    end

    if quest:step() == 0 then
        btn = me:dialog(npc, '황혼과 여명은 각각 해가 지고 뜨는 때...\n정령의 활동이 가장 활성화되는 때...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, lb = me:list(npc, '나는 이 활성화 된 정령의 힘을 모두 결집한, 황혼의 옷과 여명의 옷을 만드는 자...', { '제게도 옷을 만들어주세요.', '수고하세요.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_113_COS001B::
        btn = me:dialog(npc, '그러나 옷을 만드는 데에는 재료가 필요하다...\n정령의 힘을 담은 재료가 필요하다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_113_COS002B::
        btn = me:dialog(npc, '외부인인 그대가 과연 정령의 힘을 이해하고 그 재료가 어떤 것인지 깨달을 수 있을까...\n깨닫는 것은 그대가 할 일... 재료를 구해오는 것도 그대가 할 일... 나는 재료로 옷을 만들 뿐...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS001B
        end
        local has_any_totem = false
        for _, name in ipairs(totem_names) do
            if count_slots(me:items(name)) > 0 then
                has_any_totem = true
                break
            end
        end
        if not has_any_totem then
            me:dialog(npc, '외부인이여... 그대는 아직 깨닫지 못했다... 그대는 아직 정령의 힘을 담은 재료가 무엇인지 모르고 있다...', true, true)
            return
        end
        ::NPC_113_COS003B::
        btn = me:dialog(npc, '그렇다... 그대는 정령의 힘을 깨달았다... 토템이야말로 정령의 힘을 담은 재료...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_113_COS004B::
        btn = me:dialog(npc, '화염, 번개, 대지, 바람의토템... 모두 4가지의 토템이 3개씩 모여야 한다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS003B
        end
        ::NPC_113_COS005B::
        btn = me:dialog(npc, '그대는 비록 외부인이지만 정령의 힘을 이해했다... 노력에 대한 보답으로... 다음 재료가 무엇인지 가르쳐준다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS004B
        end
        ::NPC_113_COS006B::
        btn = me:dialog(npc, '황혼과 여명 사이에는... 정령들의 활동의 증거로 \'이슬\'이 맺힌다...\n그 이슬을 50방울 모아 마음으로부터 기원하면... 어떤 특별한 이슬... \'여신의이슬\'을 만들 수 있다...\n그 이슬 또한 옷의 재료다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS005B
        end
        btn = me:dialog(npc, '폭염도의 북쪽으로 가라... 이슬을 얻을 수 있을 것이다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS006B
        end
        quest:step(1)
        me:push_achievement(42, '투명한 이슬을 구하자.', 7, 16)
        return
    end

    if quest:step() == 1 then
        local has_all = true
        for _, name in ipairs(totem_names) do
            if count_slots(me:items(name)) < 3 then
                has_all = false
                break
            end
        end
        if not me:has_items('여신의이슬', 1) then
            has_all = false
        end
        if not has_all then
            me:dialog(npc, '아직 그대는 재료를 모두 구하지 못했다. 화염, 번개, 대지, 바람의토템... 모두 4가지의 토템이 3개씩 모여야 하며 여신의이슬이 필요하다. 이슬은 폭염도 북쪽에서 구할 수 있을 것이다...', true, true)
            return
        end

        for _, name in ipairs(totem_names) do
            local items = me:items(name)
            local removed = 0
            for slot, _ in pairs(items or {}) do
                if removed >= 3 then
                    break
                end
                me:rmitem(slot, 1, ITEM_DELETE_TYPE.GIVE)
                removed = removed + 1
            end
        end
        me:rmitem('여신의이슬', 1, ITEM_DELETE_TYPE.GIVE)
        quest:step(2)
        me:push_achievement(42, '자연의인장을 구하자.', 7, 16)

        ::NPC_113_COS007::
        btn = me:dialog(npc, '여신의이슬을 구해왔는가...\n그럼 마지막으로 자연의인장을 구하라...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_113_COS008::
        btn = me:dialog(npc, '자연의인장은 모든 정령들의 힘을 결집할 수 있는 힘을 가지는 일종의 증표다...\n자연의인장 또한 옷의 재료다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS007
        end
        btn = me:dialog(npc, '자연의인장은 이 섬을 다르시는 존재가 두 조각으로 나눠 보관하고 있다...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_113_COS008
        end
        return
    end

    if quest:step() == 2 then
        if not me:has_items('자연의인장', 1) then
            me:dialog(npc, '그대는 아직 자연의인장을 구하지 못했다..', false, false)
            return
        end
        local sel = me:list(npc, '재료를 다 모았다면, 만들고자 하는 옷의 이름을 말하라...', {
            '황혼의갑주를 주세요.(남자전사용)',
            '여명의연갑을 주세요.(여자전사용)',
            '황혼의활복을 주세요.(남자도적용)',
            '여병의도복을 주세요.(여자도적용)',
            '황혼의도포를 주세요.(남자주술사용)',
            '여명의치마를 주세요.(여자주술사용)',
            '황혼의장삼을 주세요.(남자도사용)',
            '여명의통옷을 주세요.(여자도사용)',
        })
        if sel == nil then
            return
        end
        if sel < 0 or sel >= #armor_names then
            return
        end

        local armor_name = armor_names[sel + 1]
        local code = me:exchange(
            { ['item'] = { ['자연의인장'] = 1 } },
            { ['item'] = { [armor_name] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '자연의인장을 가지고 있지 않으시군요.', false, true)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(armor_name, '을', '를') .. ' 줄 수 없네.', false, true)
            return
        end
        quest:step(3)
        me:push_achievement(42, '정령의옷을 만들었다!', 7, 16)
        me:dialog(npc, name_with(armor_name, '이', '가') .. ' 완성되었다.. ' .. armor_name .. '에 깃든 정령들이 그대를 수호할 것이다...', true, true)
        return
    end

    me:dialog(npc, '.............', true, true)
end