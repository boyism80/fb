-- npc: 백두촌포목상
local npc = require('lib.npc')
local enum = require('lib.enum')

local function do_craft(me, ch, need_items, money_needed, product_name, rate_percent)
    local req_msg = string.format('[%s]을 만들기 위해서는 재료와 금전이 필요합니다. 성공률은 %d할입니다.', product_name, rate_percent)
    local btn = me:dialog(ch, req_msg, false, true)
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end

    btn = me:dialog(ch, '실패하더라도 저를 너무 원망하지는 말아주세요...', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end
    if btn == DIALOG_RESULT.PREV then
        return do_craft(me, ch, need_items, money_needed, product_name, rate_percent)
    end

    local cost = { ['item'] = need_items, ['money'] = money_needed }
    local reward = nil
    if math.random(1, 100) <= rate_percent then
        reward = { ['item'] = { [product_name] = 1 } }
    end

    local code = me:exchange(cost, reward)
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        for item_name, need_count in pairs(need_items) do
            if not me:has_items(item_name, need_count) then
                return me:dialog(ch, name_with(item_name, '이', '가') .. ' 부족합니다.', false, true)
            end
        end
        return me:dialog(ch, '금전이 부족합니다.', false, true)
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        return me:dialog(ch, '소지품이 가득 차서 줄 수 없습니다.', false, true)
    end
    if reward == nil then
        return me:dialog(ch, '아앗... 깃털이 그만 접착제와 덕지덕지 붙어버려서 실패하고 말았어요... 죄송합니다....', false, true)
    end
    return me:dialog(ch, '자, 여기 [' .. product_name .. '] ' .. name_with(product_name, '을', '를') .. ' 만들어 드렸습니다. 예쁘게 입고 다니세요.', false, true)
end

local function craft_with_rate_intro(me, ch, product, need_items, money_needed, rate_percent, req_detail)
    local btn = me:dialog(ch, req_detail, false, true)
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end

    btn = me:dialog(ch, '성공률은 ' .. rate_percent .. '할로, 실패하더라도 저를 너무 원망하지는 말아주세요...', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end
    if btn == DIALOG_RESULT.PREV then
        return craft_with_rate_intro(me, ch, product, need_items, money_needed, rate_percent, req_detail)
    end

    return do_craft(me, ch, need_items, money_needed, product, rate_percent)
end

local function run_craft_menu(me, ch)
    local craft_sel, craft_btn = me:list(ch, '안녕하세요. 어떤것을 만들어 드릴까요?', {
        '원앙의청',
        '원앙의적',
        '표범가죽옷',
        '곰가죽옷',
        '원앙청선',
        '원앙홍선',
    }, false)
    if craft_btn == DIALOG_RESULT.QUIT or craft_sel == nil then
        return DIALOG_RESULT.NEXT
    end

    if craft_sel == 1 then
        return do_craft(me, ch, { ['올빼미의깃털'] = 100, ['청원앙의깃털'] = 10 }, 500000, '원앙의청', 50)
    elseif craft_sel == 2 then
        return do_craft(me, ch, { ['올빼미의깃털'] = 100, ['적원앙의깃털'] = 10 }, 500000, '원앙의적', 50)
    elseif craft_sel == 3 then
        local sex_name = (me:gender() == GENDER.MALE) and '남자' or '여자'
        local product = sex_name .. '표범가죽옷'
        local req_detail = '[' .. product .. '] ' .. name_with(product, '을', '를') .. ' 만들기 위해서는, [담비가죽] 100개, [흑담비가죽] 100개, [표범의가죽] 100개 그리고 금전 250만전 을 주셔야 합니다.'
        return craft_with_rate_intro(me, ch, product, { ['담비가죽'] = 100, ['흑담비가죽'] = 100, ['표범의가죽'] = 100 }, 2500000, 30, req_detail)
    elseif craft_sel == 4 then
        local sex_name = (me:gender() == GENDER.MALE) and '남자' or '여자'
        local product = sex_name .. '곰가죽옷'
        local req_detail = '[' .. product .. '] ' .. name_with(product, '을', '를') .. ' 만들기 위해서는, [담비가죽] 100개, [흑담비가죽] 30개, [백두곰가죽] 100개 그리고 금전 100만전 을 주셔야 합니다.'
        return craft_with_rate_intro(me, ch, product, { ['담비가죽'] = 100, ['흑담비가죽'] = 30, ['백두곰가죽'] = 100 }, 1000000, 30, req_detail)
    elseif craft_sel == 5 then
        return do_craft(me, ch, { ['올빼미의깃털'] = 100, ['청원앙의깃털'] = 10 }, 2000000, '원앙청선', 10)
    else
        return do_craft(me, ch, { ['올빼미의깃털'] = 100, ['적원앙의깃털'] = 10 }, 2000000, '원앙홍선', 10)
    end
end

function NPC_621(me, npc_obj)
    npc.shop(me, npc_obj, {
        menu = {
            { '물건사기', function(me, ch)
                return npc.show_buy_menu(me, ch)
            end },
            { '물건팔기', function(me, ch)
                return npc.show_sell_menu(me, ch)
            end },
            { '제작 의뢰', function(me, ch)
                return run_craft_menu(me, ch)
            end },
        },
    })
end