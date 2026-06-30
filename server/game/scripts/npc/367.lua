-- npc: 고대유물연구가
local quest = require('lib.quest')
local enum = require('lib.enum')
local BEAD_OPTIONS = {
    { "갈색구슬조각", "고대금속결정'양" },
    { "자색구슬조각", "고대금속결정'양" },
    { "청색구슬조각", "고대금속결정'음" },
    { "녹색구슬조각", "고대금속결정'음" },
}

local function run_decline(me, npc)
    me:dialog(npc, "그래, 잘 가게나.", false, false)
end

local function run_quest_accept(me, npc)
    ::NPC_367_0010::
    local btn = me:dialog(npc, "정말인가? 자네가 유물 발굴을 도와주면, 우리도 뭔가 보답을 하겠네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_367_0011::
    btn = me:dialog(npc, "무엇을 주면 좋을까... 아 그렇지! 고대유물의 조각에는 귀한 금속의 결정이 조금씩 섞여 있는 경우가 있는데, 그것을 자네에게 주겠네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_367_0010
    end
    ::NPC_367_0012::
    btn = me:dialog(npc, "요새는 남양과 남경의 굴에 있는 괴물들이 가지고 다니고 있지. 구슬조각의 형태를 하고 있다네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_367_0011
    end
    ::NPC_367_0013::
    btn = me:dialog(npc, "갈색과 자색의 구슬조각은 양의 속성을 가지고 있고, 청색과 녹색의 구슬조각은 음의 속성을 가지고 있지.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_367_0012
    end
    ::NPC_367_0014::
    btn = me:dialog(npc, "이 구슬조각에는 귀환 금속의 결정이 조금씩 섞여 있는 경우가 있는데, 그것을 자네에게 주겠네. 우리 조사단에는 필요가 없는 물건이거든.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_367_0013
    end
    ::NPC_367_0015::
    btn = me:dialog(npc, "그럼 주의하면서 구슬조각을 구해다 주겠는가? 한꺼번에 많이 가져올수록 좋다네. 그럼 잘 부탁하겠네.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_367_0014
    end
    local q = me:start_quest(quest.QUEST_ANCIENT_RELIC)
    if q == nil then
        me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
        return
    end
    me:push_achievement(41, "고대유물 연구가를 도와주자.", 7, 16)
end

local function run_bead_exchange(me, npc)
    local sel, list_btn = me:list(npc, "오, 발굴 작업은 어떻게 되었나?", {
        "구슬조각을 찾아왔습니다.",
        "아직 많이 찾지 못했습니다."
    }, true)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 1 then
        me:dialog(npc, "그렇군. 힘내서 좀 도와주게나..", false, false)
        return
    end
    if sel ~= 0 then
        return
    end

    sel, list_btn = me:list(npc, "오오 왔는가. 어떤 색깔의 구슬조각을 가져왔는가?", {
        "갈색구슬조각입니다.",
        "자색구슬조각입니다.",
        "청색구슬조각입니다.",
        "녹색구슬조각입니다."
    }, true)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 3 then
        return
    end
    local bead_item = BEAD_OPTIONS[sel + 1][1]
    local reward_item = BEAD_OPTIONS[sel + 1][2]
    local prompt = string.format("그래, %s 몇개를 %s으로 교환하시겠는가?  ", bead_item, reward_item)
    local var = me:input(npc, prompt)
    if var == nil or var == "" then
        return
    end
    local count = tonumber(var)
    if not count or count < 1 then
        return
    end
    if not me:has_items(bead_item, count) then
        local btn = me:dialog(npc, bead_item .. " " .. name_with(bead_item, '이', '가') .. " 부족하지 않은가?", false, false)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end
    local btn = me:dialog(npc, "오, 정말 가져왔군!\\n\\n이것을 결정으로 합쳐보겠네. 잠시만 기다리시게나.", false, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local reward_count = 0
    for _ = 1, count do
        if math.random(1, 100) <= 66 then
            reward_count = reward_count + 1
        end
    end
    local reward = (reward_count > 0) and { ['item'] = { [reward_item] = reward_count } } or nil
    local code = me:exchange(
        { ['item'] = { [bead_item] = count } },
        reward
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, bead_item .. " " .. name_with(bead_item, '이', '가') .. " 부족하지 않은가?", false, false)
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 " .. name_with(reward_item, '을', '를') .. " 받을 수 없네.", false, false)
        return
    end
    me:push_achievement(41, "고대유물 연구가를 도와주었다.", 7, 16)
    me:dialog(npc, "자, 여기있네.", false, false)
end

local function run_help_excavation(me, npc)
    local q = me:quest(quest.QUEST_ANCIENT_RELIC)
    if q == nil then
        run_quest_accept(me, npc)
        return
    end
    run_bead_exchange(me, npc)
end

local function run_intro_and_list(me, npc)
    ::NPC_367_0000::
    local btn = me:dialog(npc, "남경과 남양은 고대 유물의 보고야. 누구라도 세심하게 조사하면 어디에서나 유물이 나올 정도라네. 그래서 우리 조사단은 여기를 근거지로 고대유물을 발굴하고 있지.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_367_0001::
    btn = me:dialog(npc, "하지만, 요새는 괴물들이 자주 출몰하여 위험해 지원자가 없지. 그래서 항상 사람이 모자란다네. 오늘도 두명이 괴물을 보자마자 도망쳤는데, 참 걱정일세..", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_367_0000
    end
    local sel, list_btn = me:list(npc, "사람이 너무 없어서 큰일이야..", {
        "저도 발굴 작업을 돕겠습니다!",
        "열심히 찾으세요. 전 이만..."
    }, true)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto NPC_367_0001
    end
    if sel == 0 then
        run_help_excavation(me, npc)
        return
    end
    if sel == 1 then
        run_decline(me, npc)
        return
    end
end

function NPC_367(me, npc)
    run_intro_and_list(me, npc)
end
