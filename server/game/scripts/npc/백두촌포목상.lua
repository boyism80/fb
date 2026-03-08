-- @note Trash: 1_백두촌\상점.txt "백두촌포목상". List: 물건사기(callshop 0,139), 물건팔기(callshop 1,139), 제작 의뢰(6 recipes with success rate). FB: buy/sell via NPC_BUY_DIALOG/NPC_SELL_DIALOG if npc has shop data; craft: check items/money, rmitem then money then rand/mkitem.

--- @brief NPC 백두촌포목상: List (buy/sell/craft). Craft: 6 recipes (원앙의청/적, 표범가죽옷, 곰가죽옷, 원앙청선/홍선) with success rates.
--- @param me   character
--- @param npc  NPC entity
function NPC_621(me, npc)
    local sel, list_btn = me:list(npc, "안녕하세요. 무엇을 도와드릴까요?", {
        "물건사기",
        "물건팔기",
        "제작 의뢰",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            return
        end
        return
    end

    if sel == 1 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            return
        end
        return
    end

    if sel ~= 2 then
        return
    end

    -- 제작 의뢰
    local craft_sel, craft_btn = me:list(npc, "안녕하세요. 어떤것을 만들어 드릴까요?", {
        "원앙의청",
        "원앙의적",
        "표범가죽옷",
        "곰가죽옷",
        "원앙청선",
        "원앙홍선",
    }, false)
    if craft_btn == DIALOG_RESULT.QUIT or craft_sel == nil then
        return
    end

    local function do_craft(need_items, money_needed, product_name, rate_percent)
        local req_msg = string.format("[%s]을 만들기 위해서는 재료와 금전이 필요합니다. 성공률은 %d할입니다.", product_name, rate_percent)
        local btn = me:dialog(npc, req_msg, false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "실패하더라도 저를 너무 원망하지는 말아주세요...", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            return
        end
        for item_name, need_count in pairs(need_items) do
            if not me:has_items(item_name, need_count) then
                me:dialog(npc, name_with(item_name, '이', '가') .. " 부족합니다.", false, false)
                return
            end
        end
        if me:money() < money_needed then
            me:dialog(npc, "금전이 부족합니다.", false, false)
            return
        end
        local cost = { ['item'] = need_items, ['money'] = money_needed }
        local reward = nil
        if math.random(1, 100) <= rate_percent then
            reward = { ['item'] = { [product_name] = 1 } }
        end
        local code = me:exchange(cost, reward)
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "재료나 금전이 부족합니다.", false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 줄 수 없습니다.", false, false)
            return
        end
        if reward == nil then
            me:dialog(npc, "아앗... 깃털이 그만 접착제와 덕지덕지 붙어버려서 실패하고 말았어요... 죄송합니다....", false, false)
        else
            me:dialog(npc, "자, 여기 [" .. product_name .. "] " .. name_with(product_name, '을', '를') .. " 만들어 드렸습니다. 예쁘게 입고 다니세요.", false, false)
        end
    end

    if craft_sel == 0 then
        do_craft({ ["올빼미의깃털"] = 100, ["청원앙의깃털"] = 10 }, 500000, "원앙의청", 50)
    elseif craft_sel == 1 then
        do_craft({ ["올빼미의깃털"] = 100, ["적원앙의깃털"] = 10 }, 500000, "원앙의적", 50)
    elseif craft_sel == 2 then
        local sex_name = (me:gender() == GENDER.MAN) and "남자" or "여자"
        local product = sex_name .. "표범가죽옷"
        local btn = me:dialog(npc, "[" .. product .. "] " .. name_with(product, '을', '를') .. " 만들기 위해서는, [담비가죽] 100개, [흑담비가죽] 100개, [표범의가죽] 100개 그리고 금전 250만전 을 주셔야 합니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then return end
        btn = me:dialog(npc, "성공률은 3할로, 실패하더라도 저를 너무 원망하지는 말아주세요...", true, true)
        if btn == DIALOG_RESULT.QUIT then return end
        if btn == DIALOG_RESULT.PREV then return end
        do_craft({ ["담비가죽"] = 100, ["흑담비가죽"] = 100, ["표범의가죽"] = 100 }, 2500000, product, 30)
    elseif craft_sel == 3 then
        local sex_name = (me:gender() == GENDER.MAN) and "남자" or "여자"
        local product = sex_name .. "곰가죽옷"
        local btn = me:dialog(npc, "[" .. product .. "] " .. name_with(product, '을', '를') .. " 만들기 위해서는, [담비가죽] 100개, [흑담비가죽] 30개, [백두곰가죽] 100개 그리고 금전 100만전 을 주셔야 합니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then return end
        btn = me:dialog(npc, "성공률은 3할로, 실패하더라도 저를 너무 원망하지는 말아주세요...", true, true)
        if btn == DIALOG_RESULT.QUIT then return end
        if btn == DIALOG_RESULT.PREV then return end
        do_craft({ ["담비가죽"] = 100, ["흑담비가죽"] = 30, ["백두곰가죽"] = 100 }, 1000000, product, 30)
    elseif craft_sel == 4 then
        do_craft({ ["올빼미의깃털"] = 100, ["청원앙의깃털"] = 10 }, 2000000, "원앙청선", 10)
    else
        do_craft({ ["올빼미의깃털"] = 100, ["적원앙의깃털"] = 10 }, 2000000, "원앙홍선", 10)
    end
end
