local ITEM_ICE = '얼음'
local ITEM_ICE_SWORD = '얼음칼'
local ICE_SWORD_COST = 100
local ICE_SWORD_SUCCESS_CHANCE = 8  -- out of 10 (80%)

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
    ::COS001::
    local sel, btn = me:list(npc, '그래..무슨일로 날 찾아 왔는가??', {
        '얼음칼이 무엇인가요?',
        '얼음칼좀 만들어 주세요..',
        '그냥..인사차 왔어요...잘 계시죠? ^^',
    }, true)
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if btn == DIALOG_RESULT.PREV then
        return true
    end
    if sel == nil then
        return false
    end
    if sel == 0 then
        me:dialog(npc, '얼음칼은 얼음으로 만들어진 칼로써 적을 마비시킬 수 있는 능력을 가지고 있지. 단점이 있다면 수리가 안된다고 해야하나? 허허..', false, true)
        return true
    end
    if sel == 1 then
        if not me:has_items(ITEM_ICE, ICE_SWORD_COST) then
            me:dialog(npc, '얼음칼을 만들기 위해선 얼음 100개가 필요하네.', false, false)
            return true
        end
        btn = me:dialog(npc, '오..모두 모아 왔구만.. 수고했네... 어디보자.. 전체적으로 얼음이 싱글탱글 하구먼.. 허허허.. 이보게..이 좋은 얼음이면..장식품으로 써도 좋아 보이는데..정말 얼음칼로 만들고 싶나?.. 흠.. 그래..할 수 없지... 만들어 주겠네...', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return false
        end
        me:rmitem(ITEM_ICE, ICE_SWORD_COST, ITEM_DELETE_TYPE.GIVE)
        if math.random(1, 10) <= ICE_SWORD_SUCCESS_CHANCE then
            me:mkitem(ITEM_ICE_SWORD, 1)
            me:dialog(npc, '자..받게나..방금 만든 얼음칼일쎄...어떤가? 감사한가? 껄껄껄..그래..잘 가게..', false, false)
        else
            me:dialog(npc, '이런..제작 도중에 얼음이 모두 부숴저 버렸군.. 미안하네..', false, false)
        end
        return true
    end
    if sel == 2 then
        me:dialog(npc, '허허..싱겁기는..', false, true)
        return true
    end
    return true
end

local function run_spirit_summon_placeholder(me, npc)
    me:dialog(npc, '....', false, true)
end

local function run_amber_helmet_craft(me, npc, colors, has_prev)
    local options = {}
    for i = 1, #colors do
        options[i] = colors[i].list_text
    end
    local sel, btn = me:list(npc, '그럼.. 어떤 색깔의 호박보석을 가지고 왔는가?', options, has_prev)
    if btn == DIALOG_RESULT.QUIT then
        return DIALOG_RESULT.QUIT
    end
    if btn == DIALOG_RESULT.PREV then
        return DIALOG_RESULT.PREV
    end
    if sel == nil or sel < 0 or sel >= #colors then
        return DIALOG_RESULT.QUIT
    end
    local color_name = colors[sel + 1].color
    local gem_name = color_name .. '보석'
    local helmet_name = color_name .. '투구'
    local confirm_opt, confirm_btn = me:list(npc, '그래.. ' .. gem_name .. '으로 어떤 아이템을 만들텐가?', { color_name .. '투구입니다.' }, false)
    if confirm_btn == DIALOG_RESULT.QUIT then
        return DIALOG_RESULT.QUIT
    end
    if confirm_opt ~= 0 then
        return DIALOG_RESULT.NEXT
    end
    if not me:has_items({ [gem_name] = 1, [ITEM_IRON_DUST] = HELMET_IRON_DUST_COUNT }) then
        return me:dialog(npc, '자네 혹시 재료를 모르는 것인가? ' .. gem_name .. '과 쇠가루 2개를 가지고 와야 ' .. helmet_name .. '를 만들어 줄 수 있네.', false, true)
    end
    me:rmitem({ [gem_name] = 1, [ITEM_IRON_DUST] = HELMET_IRON_DUST_COUNT }, ITEM_DELETE_TYPE.GIVE)
    me:mkitem(helmet_name, 1)
    return me:dialog(npc, gem_name .. '으로 ' .. helmet_name .. '를 만들어주었네.', false, true)
end

function NPC_156(me, npc)
    ::COS001::
    local sel, btn = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {
        '얼음칼만들기',
        '초혼술방법',
        '호박투구만들기',
        '연호박투구만들기',
        '황금호박투구만들기',
    }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end
    if sel == 0 then
        if run_ice_sword(me, npc) == false then
            return
        end
    elseif sel == 1 then
        run_spirit_summon_placeholder(me, npc)
    elseif sel == 2 then
        local r = run_amber_helmet_craft(me, npc, AMBER_HELMET_COLORS, true)
        if r == DIALOG_RESULT.QUIT then
            return
        end
        if r == DIALOG_RESULT.PREV then
            goto COS001
        end
    elseif sel == 3 then
        local r = run_amber_helmet_craft(me, npc, LIGHT_AMBER_HELMET_COLORS, true)
        if r == DIALOG_RESULT.QUIT then
            return
        end
        if r == DIALOG_RESULT.PREV then
            goto COS001
        end
    elseif sel == 4 then
        local r = run_amber_helmet_craft(me, npc, GOLD_AMBER_HELMET_COLORS, true)
        if r == DIALOG_RESULT.QUIT then
            return
        end
        if r == DIALOG_RESULT.PREV then
            goto COS001
        end
    end
    goto COS001
end