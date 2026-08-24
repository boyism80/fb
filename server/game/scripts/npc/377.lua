-- npc: 중국대장장이
local enum = require('lib.enum')
local npc = require('lib.npc')

local YIN_FRAGMENT = "고대금속조각'음"
local YANG_FRAGMENT = "고대금속조각'양"

local YIN_RECIPES = {
    { count = 10, reward = '비취의목걸이', success = '축하하네. 이것을 정말로 만들어보게 되다니 나도 영광일세.' },
    { count = 30, reward = '연청투구', success = '축하하네! 음의 기운을 띄고 있어서 약간 푸르스름한 기운이 도는 투구로군. 투구에서 뿜어져 나오는 기운이 심상치 않은데..' },
    { count = 50, reward = '외강의복', success = '축하하네. 이것을 정말로 만들어보게 되다니 나도 영광일세.' },
}

local YANG_RECIPES = {
    { count = 10, reward = '수정의목걸이', success = '축하하네. 이것을 정말로 만들어보게 되다니 나도 영광일세.' },
    { count = 30, reward = '연홍투구', success = '축하하네! 음의 기운을 띄고 있어서 약간 푸르스름한 기운이 도는 투구로군. 투구에서 뿜어져 나오는 기운이 심상치 않은데..' },
    { count = 50, reward = '내강의복', success = '축하하네. 이것을 정말로 만들어보게 되다니 나도 영광일세.' },
}

local function craft_fragment(me, npc_obj, fragment_name, recipes, menu_prompt)
    local labels = {}
    for i = 1, #recipes do
        labels[i] = string.format('%s를 만들어 주세요.', recipes[i].reward)
    end

    local selected = me:list(npc_obj, menu_prompt, labels)
    if selected == nil then
        return
    end

    local recipe = recipes[selected]
    if recipe == nil then
        return
    end

    if not me:has_items({ [fragment_name] = recipe.count }) then
        me:dialog(npc_obj, string.format('적어도 %d개정도는 있어야 뭔가 말들어볼 수 있을 것 같은데 말이야.. 좀 부족하군.', recipe.count), { prev = false, next = true })
        return
    end

    local code = me:exchange(
        { ['item'] = { [fragment_name] = recipe.count } },
        { ['item'] = { [recipe.reward] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc_obj, string.format('적어도 %d개정도는 있어야 뭔가 말들어볼 수 있을 것 같은데 말이야.. 좀 부족하군.', recipe.count), { prev = false, next = true })
    elseif code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc_obj, '소지품이 가득 차서 ' .. name_with(recipe.reward, '을', '를') .. ' 드리지 못합니다.', { prev = false, next = true })
    else
        me:dialog(npc_obj, recipe.success, { prev = false, next = true })
    end
end

local function craft_yin_and_yang_blade(me, npc_obj)
    if not me:has_items({ [YIN_FRAGMENT] = 50, [YANG_FRAGMENT] = 50 }) then
        me:dialog(npc_obj, "음과 양의 금속조각을 섞겠다고? 농담 말게나. 이 정도 양을 섞겠다고 난리를 치다가는 힘들여 모은 이 금속조각들을 전부 잃게 될 수도 있다네. 혹시 음과 양이 각각 50개 정도 있다면 모를까...", { prev = false, next = true })
        return
    end

    local btn = me:dialog(npc_obj, "고대금속조각'양 50개와 고대금속조각'음 50개를 사용해서 음양도를 만들 수 있다네.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local selected = me:list(npc_obj, "고대금속조각'양 50개와 고대금속조각'음 50개를 사용해서 음양도를 만들겠는가?", {
        '네, 만들어 주십시요!',
        '좀 더 생각해보겠습니다.',
    }, { prev = true })
    if selected == nil then
        return
    end
    if selected == 2 then
        return
    end

    local cost = { ['item'] = { [YIN_FRAGMENT] = 50, [YANG_FRAGMENT] = 50 } }
    local reward = nil
    if math.random(1, 100) <= 60 then
        reward = { ['item'] = { ['음양도'] = 1 } }
    end

    local code = me:exchange(cost, reward)
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc_obj, "음과 양의 금속조각을 섞겠다고? 농담 말게나. 이 정도 양을 섞겠다고 난리를 치다가는 힘들여 모은 이 금속조각들을 전부 잃게 될 수도 있다네. 혹시 음과 양이 각각 50개 정도 있다면 모를까...", { prev = false, next = true })
    elseif code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc_obj, '소지품이 가득 차서 ' .. name_with('음양도', '을', '를') .. ' 드리지 못합니다.', { prev = false, next = true })
    elseif reward == nil then
        me:dialog(npc_obj, '아앗! 미안하게 되었네.. 실패하고 말았군....... 이를 어쩌나...', { prev = false, next = true })
    else
        me:dialog(npc_obj, '오오오! 이것은!! 축하하네! 음양도는 굉장히 강력한 무기라는 것 밖에.. 나도 아무것도 알지 못한다네. 직접 사용해보는게 좋겠군.', { prev = false, next = true })
    end
end

local function show_ancient_craft_menu(me, npc_obj)
    local selected = me:list(npc_obj, '무슨 일로 찾아오셨습니까?', {
        "고대금속조각'음을 가공해주세요.",
        "고대금속조각'양을 가공해주세요.",
        '둘을 합쳐서 가공해주세요.',
    })
    if selected == nil then
        return
    end

    if selected == 1 then
        craft_fragment(me, npc_obj, YIN_FRAGMENT, YIN_RECIPES, "고대금속조각'음을 이용하여 무엇을 만들겠는가?")
    elseif selected == 2 then
        craft_fragment(me, npc_obj, YANG_FRAGMENT, YANG_RECIPES, "고대금속조각'양을 이용하여 무엇을 만들겠는가?")
    elseif selected == 3 then
        craft_yin_and_yang_blade(me, npc_obj)
    end
end

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '안녕하세요. 어떻게 오셨나요?',
            menu = {
                { '물건 고치기', function(ch, shop_npc)
                    return npc.show_repair_menu(ch, shop_npc)
                end },
                { '모두 고치기', function(ch, shop_npc)
                    return npc.show_repair_all_menu(ch, shop_npc)
                end },
                { '고대 유물 가공', function(ch, shop_npc)
                    show_ancient_craft_menu(ch, shop_npc)
                    return DIALOG_RESULT.NEXT
                end },
            },
        })
    end,
}
