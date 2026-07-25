-- npc: 시계장인
local enum = require('lib.enum')

local CLOCK_PRICE = 10000
local CLOCK_ITEM_NAME = '장인의회중시계'
local BATTERY_PRICE = 5000
local BATTERY_ITEM_NAME = '건전지'

local function run_clock_purchase(me, npc)
    ::NPC_250_0002::
    local btn = me:dialog(npc, '자네도 내가 만든 시계를 구입하고 싶어서 찾아온 게로구만? 그렇다면 잘 찾아왔네.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_250_0003::
    btn = me:dialog(npc, '그렇다면 잘 찾아왔구만. 하지만 공짜로 줄 순 없지.. 1만전만 낸다면야 생각은 해 보겠다만..', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_250_0002
    end
    local confirm = me:list(npc, '1만전을 내고 나에게 시계를 사겠나?', { '네, 살래요.', '안살래요.' }, { prev = false })
    if confirm == nil or confirm ~= 1 then
        return
    end
    local code = me:exchange(
        { ['money'] = CLOCK_PRICE },
        { ['item'] = { [CLOCK_ITEM_NAME] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '아니.. 돈도 없이 내 시계를 살 순 없지.', { prev = false, next = false })
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 시계를 줄 수 없네.', { prev = false, next = false })
        return
    end
    me:dialog(npc, '여기 있네. 요긴하게 잘 사용하게나.', { prev = false, next = false })
end

local function run_battery_purchase(me, npc)
    local stock = property("clock_time_item")
    if stock == nil then
        stock = 0
    end

    local btn = me:dialog(npc, '안녕하신가? 나는 매일 오후 6시마다 나의 시계에 사용할수 있는 건전지를 10개씩 매입해온다네.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local msg = string.format('건전지는 하나당 %d전이고, 현재 건전지는 %d개 있다네. 구입할텐가?', BATTERY_PRICE, stock)
    local choice = me:list(npc, msg, { '네, 건전지 주세요.', '아니요, 아직 시계의 수명이 넉넉해서요.' }, { prev = false })
    if choice == nil or choice ~= 1 then
        return
    end

    if stock == 0 then
        me:dialog(npc, '지금은 다 떨어졌네.. 다음에 다시 오게나.', { prev = false, next = false })
        return
    end
    local code = me:exchange(
        { ['money'] = BATTERY_PRICE },
        { ['item'] = { [BATTERY_ITEM_NAME] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '돈이 부족하구만.. 나의 건전지는 하나당 5000전이라네.', { prev = false, next = false })
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 건전지를 줄 수 없네.', { prev = false, next = false })
        return
    end
    property("clock_time_item", stock - 1)
    me:dialog(npc, '자네 생각보다 운이 좋은걸? 여기 건전기 가져가게나.', { prev = false, next = false })
end

return {
    on_click = function(me, npc)
        ::NPC_250_0001::
        local sel = me:list(npc, '안녕하신가? 나에겐 어쩐일로 왔는가?', { '시계 구입', '건전지 구입' }, { prev = false })
        if sel == nil then
            return
        end

        if sel == 1 then
            run_clock_purchase(me, npc)
        elseif sel == 2 then
            run_battery_purchase(me, npc)
        end
    end
}
