-- npc: 백의장인
local enum = require('lib.enum')

local DRAGON_AWAKEN_RATES = { 20, 8, 2 }

local DRAGON_WEAPON_LINES = {
    {
        type_name = '용마',
        options = { '용마제육검', '용마제칠검', '용마제팔검' },
        success = { '용마제칠검', '용마제팔검', '용마제구검' },
        failed   = { nil, '용마제칠검(손상)', '용마제팔검(손상)' },
    },
    {
        type_name = '용천',
        options = { '용천제육검', '용천제칠검', '용천제팔검' },
        success = { '용천제칠검', '용천제팔검', '용천제구검' },
        failed   = { nil, '용천제칠검(손상)', '용천제팔검(손상)' },
    },
    {
        type_name = '용랑',
        options = { '용랑제육봉', '용랑제칠봉', '용랑제팔봉' },
        success = { '용랑제칠봉', '용랑제팔봉', '용랑제구봉' },
        failed   = { nil, '용랑제칠봉(손상)', '용랑제팔봉(손상)' },
    },
    {
        type_name = '용겸',
        options = { '용겸제육봉', '용겸제칠봉', '용겸제팔봉' },
        success = { '용겸제칠봉', '용겸제팔봉', '용겸제구봉' },
        failed   = { nil, '용겸제칠봉(손상)', '용겸제팔봉(손상)' },
    },
}

local AMBER_COLORS = {
    { list_text = '황호박별입니다.',  color = '황호박' },
    { list_text = '적호박별입니다.',  color = '적호박' },
    { list_text = '녹호박별입니다.',  color = '녹호박' },
    { list_text = '청호박별입니다.',  color = '청호박' },
    { list_text = '회호박별입니다.',  color = '회호박' },
    { list_text = '갈호박별입니다.',  color = '갈호박' },
    { list_text = '자호박별입니다.',  color = '자호박' },
}

local LIGHT_AMBER_COLORS = {
    { list_text = '연녹호박별입니다.', color = '연녹호박' },
    { list_text = '연청호박별입니다.', color = '연청호박' },
    { list_text = '연갈호박별입니다.', color = '연갈호박' },
    { list_text = '연자호박별입니다.', color = '연자호박' },
}

local WEAPON_NAMES = { '별검', '별도', '별봉', '별곤' }

local function show_no_ice_dialog(me, npc)
    me:dialog(npc, '아이고.. 더워.. 본좌는 지금 더워서 아무것도 못하겠으니 그만 돌아가거라.', { prev = false, next = false })
    return true
end

local function run_dragon_weapon_awaken(me, npc)
    ::NPC_140_0001::
    local btn = me:dialog(npc, '상급 용무기를 각성시키러 왔다고? 그렇다면 잘 찾아왔네.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_140_0002::
    btn = me:dialog(npc, '이제까지 나의 제자 천금장인에게 용무기를 맡기느라 수고가 많았네.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_140_0001
    end
    ::NPC_140_0003::
    btn = me:dialog(npc, '하지만 아무리 나라도 가끔 실수할 때가 있는법.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_140_0002
    end
    ::NPC_140_0004::
    local sel, list_btn = me:list(npc, '그래. 그래도 나에게 용무기 각성을 맡기겠나?', { '네. 용무기를 각성해주세요.' }, { prev = true })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return false
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto NPC_140_0003
    end
    if sel ~= 1 then
        return false
    end
    ::NPC_140_0005::
    local type_options = {}
    for _, line in ipairs(DRAGON_WEAPON_LINES) do
        type_options[#type_options + 1] = line.type_name
    end
    local type_sel, type_btn = me:list(npc, '좋아. 각오가 대단하군. 그럼 각성시킬 용무기를 고르게.', type_options, { prev = true })
    if type_btn == DIALOG_RESULT.QUIT or type_sel == nil then
        return false
    end
    if type_btn == DIALOG_RESULT.PREV then
        goto NPC_140_0004
    end
    if type_sel < 1 or type_sel > #DRAGON_WEAPON_LINES then
        return false
    end
    local line = DRAGON_WEAPON_LINES[type_sel]
    ::NPC_140_0006::
    local grade_sel, grade_btn = me:list(npc, '어떤 무기를 각성할건가?', line.options, { prev = true })
    if grade_btn == DIALOG_RESULT.QUIT or grade_sel == nil then
        return false
    end
    if grade_btn == DIALOG_RESULT.PREV then
        goto NPC_140_0005
    end
    if grade_sel < 1 or grade_sel > 3 then
        return false
    end
    local check_item = line.options[grade_sel]
    local success_item = line.success[grade_sel]
    local failed_item = line.failed[grade_sel]
    local rate = DRAGON_AWAKEN_RATES[grade_sel]
    if not me:has_items({ ['얼음'] = 1, ['은나무가지'] = 1, [check_item] = 1 }) or me:base_hp() < 1000 then
        me:dialog(npc, '자네는 아직 용무기를 각성시킬 준비가 모자라군.', { prev = false, next = false })
        return true
    end
    local roll = math.random(1, 100)
    me:rmitem({ ['얼음'] = 1, ['은나무가지'] = 1, [check_item] = 1 }, ITEM_DELETE_TYPE.GIVE)
    me:base_hp(me:base_hp() - 1000, false)
    me:hp(me:maxhp(), false)
    me:update()
    if roll <= rate then
        broadcast(string.format('%s님이 %s 강화에 성공하셨습니다.', me:name(), check_item), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
        me:mkitem(success_item, 1)
        me:dialog(npc, '축하하네. 하늘이 자네를 ' .. success_item .. '의 주인으로 인정하는군', { prev = false, next = false })
    else
        broadcast(string.format('%s님이 %s 강화에 실패하셨습니다.', me:name(), check_item), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
        if failed_item then
            me:mkitem(failed_item, 1)
        end
        me:dialog(npc, '이런.. 용무기가 온도를 이기지 못하고 부숴저 버렸군..', { prev = false, next = false })
    end
    return true
end

local function run_amber_weapon_craft(me, npc, colors)
    local color_opts = {}
    for _, c in ipairs(colors) do
        color_opts[#color_opts + 1] = c.list_text
    end
    local color_sel, color_btn = me:list(npc, '그럼.. 어떤 색깔의 호박별을 가지고 왔는가?', color_opts, { prev = true })
    if color_btn == DIALOG_RESULT.PREV then
        return DIALOG_RESULT.PREV
    end
    if color_sel == nil or color_sel < 1 or color_sel > #colors then
        return DIALOG_RESULT.QUIT
    end
    local color_name = colors[color_sel].color
    local weapon_opts = {}
    for _, w in ipairs(WEAPON_NAMES) do
        weapon_opts[#weapon_opts + 1] = color_name .. w .. '입니다.'
    end
    local weapon_sel, weapon_btn = me:list(npc, '그래.. ' .. name_with(color_name .. '별', '으로', '로') .. ' 어떤 아이템을 만들텐가?', weapon_opts, { prev = false })
    if weapon_btn == DIALOG_RESULT.PREV then
        return DIALOG_RESULT.PREV
    end
    if weapon_sel == nil or weapon_sel < 1 or weapon_sel > #WEAPON_NAMES then
        return DIALOG_RESULT.QUIT
    end
    local star_item = color_name .. '별'
    local result_name = color_name .. WEAPON_NAMES[weapon_sel]
    local code = me:exchange(
        { ['item'] = { [star_item] = 1, ['죽은지네'] = 1 } },
        { ['item'] = { [result_name] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '재료를 다시 한번 살펴보게. ' .. color_name .. '별과 죽은지네가 있어야 제작할 수 있다네.', { prev = false, next = false })
        return DIALOG_RESULT.NEXT
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 ' .. name_with(result_name, '을', '를') .. ' 받을 수 없네.', { prev = false, next = false })
        return DIALOG_RESULT.NEXT
    end
    me:dialog(npc, name_with(color_name .. '별', '으로', '로') .. ' ' .. color_name .. WEAPON_NAMES[weapon_sel] .. ' 만들어주었네.', { prev = false, next = false })
    return DIALOG_RESULT.NEXT
end

return {
    ON_CLICK = function(me, npc)
        if not me:has_items('얼음', 1) then
            show_no_ice_dialog(me, npc)
            return
        end
        ::NPC_140_START::
        local main_opt = me:list(npc, '안녕하신가? 무엇을 도와줄까?', {
            '상급 용무기를 각성시켜 주세요.',
            '호박무기만들기',
            '연호박무기만들기',
        }, { prev = false })
        if main_opt == nil then
            return
        end
        if main_opt == 1 then
            if run_dragon_weapon_awaken(me, npc) == false then
                return
            end
        elseif main_opt == 2 then
            local r = run_amber_weapon_craft(me, npc, AMBER_COLORS)
            if r == DIALOG_RESULT.QUIT then
                return
            end
            if r == DIALOG_RESULT.PREV then
                goto NPC_140_START
            end
        elseif main_opt == 3 then
            local r = run_amber_weapon_craft(me, npc, LIGHT_AMBER_COLORS)
            if r == DIALOG_RESULT.QUIT then
                return
            end
            if r == DIALOG_RESULT.PREV then
                goto NPC_140_START
            end
        end
    end
}
