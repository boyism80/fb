-- Castle NPC menus: 문파성선생, 신수의옷, 대장장이, 수련스승, 일반 상점.
local npc = require('lib.npc')
local castle_lib = require('lib.castle')

local M = {}

local DIVINE_BEAST_BY_CASTLE_MAP = {
    ['주작의눈'] = DIVINE_BEAST.VERMILION_BIRD,
    ['현무의눈'] = DIVINE_BEAST.BLACK_TORTOISE,
    ['청룡의눈'] = DIVINE_BEAST.AZURE_DRAGON,
    ['백호의눈'] = DIVINE_BEAST.WHITE_TIGER,
    ['주작의성'] = DIVINE_BEAST.VERMILION_BIRD,
    ['현무의성'] = DIVINE_BEAST.BLACK_TORTOISE,
    ['청룡의성'] = DIVINE_BEAST.AZURE_DRAGON,
    ['백호의성'] = DIVINE_BEAST.WHITE_TIGER,
}

local SECRETARIES = {
    { '장안성비서', 100 },
    { '부여성비서', 100 },
    { '국내성비서', 100 },
    { '귀환비서', 50 },
    { '신부여성비서', 500 },
}

local ARMOR_SELL_CATEGORIES = {
    { '전사용 갑주류', 11 },
    { '전사용 비늘갑주류', 12 },
    { '도적용 도복류', 13 },
    { '도적용 갑옷류', 14 },
    { '주술사용 치마류', 15 },
    { '주술사용 도포류', 16 },
    { '도사용 장삼류', 17 },
}

local WEAPON_SELL_CATEGORIES = {
    { '무기류', 22 },
}

local CLOTH_SELL_CATEGORIES = {
    { '옷감류', 19 },
}

local FOOD_SELL_CATEGORIES = {
    { '음식류', 20 },
}

local function map_divine_beast(me)
    local map = me:map()
    if map == nil then
        return nil
    end
    local name = map:model():name()
    local divine = DIVINE_BEAST_BY_CASTLE_MAP[name]
    if divine ~= nil then
        return divine
    end
    return select(2, castle_lib.totem_of_map(map))
end

local function is_clan_master(me)
    local clan = me:clan()
    if clan == nil then
        return false, nil
    end
    for _, member in pairs(clan:members()) do
        if member:role() == CLAN_ROLE.MASTER and member:name() == me:name() then
            return true, clan
        end
    end
    return false, clan
end

local function owns_this_castle(me)
    local clan = me:clan()
    local divine = map_divine_beast(me)
    if clan == nil or divine == nil then
        return false, clan, divine
    end
    local castle_obj = castle(divine)
    if castle_obj == nil or castle_obj:owner_clan() ~= clan:id() then
        return false, clan, divine
    end
    return true, clan, divine
end

local function buy_clan_sword(me, npc_obj)
    local is_master, clan = is_clan_master(me)
    if clan == nil then
        me:dialog(npc_obj, '가입된 문파가 없습니다.', { prev = false, next = false })
        return
    end
    if not is_master then
        me:dialog(npc_obj, '너는 문파장이 아니구나!', { prev = false, next = false })
        return
    end

    local owns, _, divine_beast = owns_this_castle(me)
    if not owns then
        me:dialog(npc_obj, '이 성을 가진 문파장에게만 자격이 있다네.', { prev = false, next = false })
        return
    end

    local weapon_name = castle_lib.WEAPON_NAME_BY_DIVINE_BEAST[divine_beast]
    if castle_lib.has_item_named(me, weapon_name) then
        me:dialog(npc_obj, '자네는 이미 이 성을 대표하는 검을 가지고 있는 것 같군..', { prev = false, next = false })
        return
    end

    local model = name2item(weapon_name)
    if model == nil then
        return
    end
    local price = model:price()

::BUY_INTRO_1::
    local button = me:dialog(npc_obj, '이 성을 가진 주인이라면 마땅히 그 힘을 가질 자격이 있지.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end

::BUY_INTRO_2::
    button = me:dialog(npc_obj, '그러나 그 힘을 얻기 위해서는 그에 마땅한 댓가로 따르는 법...', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BUY_INTRO_1
    end

::BUY_CONFIRM::
    local OPT_YES = '네. 지금 당장 주세요!'
    local OPT_NO = '아니요. 돈이 없네요...'
    local selected, list_btn = me:list(npc_obj, string.format('그래, 금전 %d전을 댓가로 문파검을 구입하겠는가?', price), { OPT_YES, OPT_NO }, { prev = true })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto BUY_INTRO_2
    end
    if selected ~= 1 then
        return
    end

    if me:money() < price then
        button = me:dialog(npc_obj, '감히 어디서 거짓말인가!!', { prev = true, next = false })
        if button == DIALOG_RESULT.PREV then
            goto BUY_CONFIRM
        end
        return
    end

    if me:mkitem(weapon_name, 1) == nil then
        button = me:dialog(npc_obj, '소지품이 가득 찼습니다.', { prev = true, next = false })
        if button == DIALOG_RESULT.PREV then
            goto BUY_CONFIRM
        end
        return
    end

    me:money(me:money() - price)
    button = me:dialog(npc_obj, '이 검은 극도로 강한 기운을 잡아놓아 무척이나 불안하다네.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc_obj, '죽으면 그 힘이 모두 사라지게 되니 조심해서 쓰시게나.', { prev = true, next = false })
end

local function buy_secretary(me, npc_obj)
    local labels = {}
    for i, pair in ipairs(SECRETARIES) do
        labels[i] = string.format('%s (%d전)', pair[1], pair[2])
    end

    local selected, button = me:list(npc_obj, '어떤 비서를 구입하시겠습니까?', labels)
    if button == DIALOG_RESULT.QUIT or selected == nil then
        return
    end

    local item_name, price = table.unpack(SECRETARIES[selected])
    if name2item(item_name) == nil then
        me:dialog(npc_obj, '준비중인 물품입니다.', { prev = false, next = false })
        return
    end
    if me:money() < price then
        me:dialog(npc_obj, '돈이 모자랍니다.', { prev = false, next = false })
        return
    end
    if me:mkitem(item_name, 1) == nil then
        me:dialog(npc_obj, '공간이 부족합니다.', { prev = false, next = false })
        return
    end
    me:money(me:money() - price)
    me:dialog(npc_obj, string.format('%s %d전에 팔았습니다.', name_with(item_name), price), { prev = false, next = false })
end

local function revive_ghost(me, npc_obj)
    if me:state() ~= STATE.GHOST and me:hp() > 0 then
        me:dialog(npc_obj, '자네는 잘 살아 있는데?', { prev = false, next = false })
        return
    end

    me:dialog(npc_obj, '쯧쯧... 그 사정이 딱하여 딱 한번 도와줄테니, 다시는 이런 일이 없도록 하거라!', { prev = false, next = true })
    me:hp(1)
    me:state(STATE.NORMAL)
    npc_obj:chat('영은 생을 얻을지어니..')
    me:dialog(npc_obj, '영은 생을 얻을지어니..', { prev = false, next = false })
end

local function clan_vault(me, npc_obj)
    local is_master, clan = is_clan_master(me)
    if clan == nil then
        me:dialog(npc_obj, '가입된 문파가 없습니다.', { prev = false, next = false })
        return
    end

    local OPT_QUERY = '문파자금 조회'
    local OPT_DEPOSIT = '문파자금 입금'
    local OPT_WITHDRAW = '문파자금 출금'
    local menu = { OPT_QUERY, OPT_DEPOSIT }
    if is_master then
        table.insert(menu, OPT_WITHDRAW)
    end

    local selected, button = me:pursuit(npc_obj, string.format('현재 문파자금은 %d전입니다. 무엇을 하시겠습니까?', clan:money()), menu)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if selected == OPT_QUERY then
        me:dialog(npc_obj, string.format('문파자금은 %d전입니다.', clan:money()), { prev = false, next = false })
        return
    end

    if selected == OPT_DEPOSIT then
        local amount = me:input(npc_obj, '얼마를 입금하시겠습니까?')
        if amount == nil then
            return
        end
        amount = tonumber(amount)
        if amount == nil or amount <= 0 then
            me:dialog(npc_obj, '금액이 올바르지 않습니다.', { prev = false, next = false })
            return
        end
        if me:money() < amount then
            me:dialog(npc_obj, '보유 금전이 부족합니다.', { prev = false, next = false })
            return
        end
        me:money(me:money() - amount)
        local _, err = clan:money(amount)
        if err ~= nil then
            me:money(me:money() + amount)
            me:dialog(npc_obj, '문파자금 입금에 실패했습니다.', { prev = false, next = false })
            return
        end
        me:dialog(npc_obj, string.format('%d전을 문파자금에 입금했습니다.', amount), { prev = false, next = false })
        return
    end

    if selected == OPT_WITHDRAW then
        local intro = me:dialog(npc_obj, '가지고 있는 성의 자금이 있다면, 언제든 찾아 쓸 수가 있지. 그래, 자네는 문파의 문주인가?', { prev = false, next = true })
        if intro == DIALOG_RESULT.QUIT then
            return
        end
        if not is_master then
            me:dialog(npc_obj, '문주가 아니라면 찾을 수 없네.', { prev = false, next = false })
            return
        end
        if clan:money() == 0 then
            me:dialog(npc_obj, '자네의 문파는 보유한 자금이 없다네.', { prev = false, next = false })
            return
        end
        local amount = me:input(npc_obj, string.format('그대의 문파 [%s]가 보유한 자금은 [%d]전 이라네. 제가 찾을 금액은?', clan:name(), clan:money()))
        if amount == nil then
            return
        end
        amount = tonumber(amount)
        if amount == nil or amount < 0 then
            me:dialog(npc_obj, '장난이 과하군.', { prev = false, next = false })
            return
        end
        if amount == 0 then
            return
        end
        if clan:money() < amount then
            me:dialog(npc_obj, '그렇게 많이 찾을 수는 없다네.', { prev = false, next = false })
            return
        end
        local _, err = clan:money(-amount)
        if err ~= nil then
            me:dialog(npc_obj, '문파자금 출금에 실패했습니다.', { prev = false, next = false })
            return
        end
        me:money(me:money() + amount)
        me:message(string.format('문파자금 %d전 찾았습니다.', amount), MESSAGE_TYPE.NOTIFY)
        me:message(string.format('금전 %d전 받았습니다.', amount), MESSAGE_TYPE.NOTIFY)
        me:dialog(npc_obj, string.format('자, 여기있네. 자네에게 문파자금 [%d]전을 줄테니 잘 쓰시게나.', amount), { prev = false, next = false })
    end
end

function M.on_sensei_click(me, npc_obj)
    local OPT_SECRETARY = '비서 구입'
    local OPT_SWORD = '문파검 구입'
    local OPT_REVIVE = '부활'
    local OPT_VAULT = '문파자금'
    local selected, button = me:pursuit(npc_obj, '성을 관리하기란 쉬운 일이 아니지. 그래, 무엇을 원하는가?', {
        OPT_SECRETARY, OPT_SWORD, OPT_REVIVE, OPT_VAULT
    })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if selected == OPT_SECRETARY then
        buy_secretary(me, npc_obj)
    elseif selected == OPT_SWORD then
        buy_clan_sword(me, npc_obj)
    elseif selected == OPT_REVIVE then
        revive_ghost(me, npc_obj)
    elseif selected == OPT_VAULT then
        clan_vault(me, npc_obj)
    end
end

function M.buy_clothes(me, npc_obj)
    local owns, clan, divine_beast = owns_this_castle(me)
    if clan == nil then
        me:dialog(npc_obj, '가입된 문파가 없습니다.', { prev = false, next = false })
        return
    end
    if not owns then
        me:dialog(npc_obj, '오로지 성을 점령한 문파에게만 권리가 주어진다네.', { prev = false, next = false })
        return
    end

    for _, clothes_name in ipairs(castle_lib.ALL_CLOTHES_NAMES) do
        if castle_lib.has_item_named(me, clothes_name) then
            me:dialog(npc_obj, '당신에게서는 이미 강력한 신수의 기운이 느껴지는군요. 기운이 모두 소진되거든 다시 돌아 오십시오...', { prev = false, next = false })
            return
        end
    end

    local clothes_name = castle_lib.CLOTHES_NAME_BY_DIVINE_BEAST[divine_beast]
    if name2item(clothes_name) == nil then
        me:dialog(npc_obj, '준비중인 물품입니다.', { prev = false, next = false })
        return
    end

    local price = 50000
    if me:dialog(npc_obj, string.format('안녕하십니까 %s님.', me:name()), { prev = false, next = true }) == DIALOG_RESULT.QUIT then
        return
    end
    if me:dialog(npc_obj, string.format('[%s]을 구하러 오신 것을 알고 있습니다. 하지만 주의하셔야 할 것이 있습니다.', clothes_name), { prev = true, next = true }) == DIALOG_RESULT.QUIT then
        return
    end
    if me:dialog(npc_obj, string.format('[%s]은 어디까지나 신수의 힘을 잠시 빌려 만든 옷. 일정 시간이 지나면 그 힘이 다하게 됩니다.', clothes_name), { prev = true, next = true }) == DIALOG_RESULT.QUIT then
        return
    end
    if me:dialog(npc_obj, '그 점을 기억하시기 바랍니다.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
        return
    end

    local OPT_YES = '네. 주세요.'
    local OPT_NO = '너무 비싸요.'
    local selected = me:pursuit(npc_obj, string.format('[%s]을 구입하시겠습니까? 비용은 %d전 입니다.', clothes_name, price), { OPT_YES, OPT_NO })
    if selected == OPT_NO then
        me:dialog(npc_obj, '그럼 더 신중히 생각해 보신 뒤 오시기 바랍니다.', { prev = false, next = false })
        return
    end
    if selected ~= OPT_YES then
        return
    end
    if me:money() < price then
        me:dialog(npc_obj, '보유하신 금전이 부족하시군요.', { prev = false, next = false })
        return
    end
    if me:mkitem(clothes_name, 1) == nil then
        me:dialog(npc_obj, '공간이 부족합니다.', { prev = false, next = false })
        return
    end
    me:money(me:money() - price)
    me:message(string.format('%s 받았습니다.', clothes_name), MESSAGE_TYPE.NOTIFY)
    me:dialog(npc_obj, string.format('여기 %s을 드렸습니다. 잘 사용하시기 바랍니다...', clothes_name), { prev = false, next = false })
end

function M.repair_dragon_weapon(me, npc_obj)
    if me:dialog(npc_obj, '용무기를 수리하고 싶다고?\n\n돈이 만만치 않을텐데..', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
        return
    end

    local weapon = me:weapon()
    if weapon == nil then
        me:dialog(npc_obj, '착용중인 무기가 없습니다.', { prev = false, next = false })
        return
    end

    local name = weapon:model():name()
    if not name:find('용', 1, true) then
        me:dialog(npc_obj, '용무기만 수리할 수 있다네.', { prev = false, next = false })
        return
    end

    local model = weapon:model()
    local max_dur = model:durability()
    local cur_dur = weapon:durability()
    if cur_dur >= max_dur then
        me:dialog(npc_obj, '이미 멀쩡한 무기일세.', { prev = false, next = false })
        return
    end

    local unit_price = model:repair_price() or 0
    local price = math.floor(unit_price * (max_dur - cur_dur) * 2)
    if price < 1 then
        price = 1
    end

    local OPT_YES = '네 맞습니다.'
    local OPT_NO = '아니오, 착용후 다시 올게요.'
    local selected = me:pursuit(npc_obj, string.format('지금부터 자네 손에 들고있는 용무기를 고쳐주겠네. 자네가 고칠 무기는 [%s]가 맞는가? 수리비 %d전.', name, price), { OPT_YES, OPT_NO })
    if selected ~= OPT_YES then
        return
    end
    if me:money() < price then
        me:dialog(npc_obj, '돈이 모자랍니다.', { prev = false, next = false })
        return
    end
    me:money(me:money() - price)
    weapon:durability(max_dur)
    me:dialog(npc_obj, '용무기를 수리했습니다.', { prev = false, next = false })
end

function M.on_armor_shop_click(me, npc_obj)
    npc.shop(me, npc_obj, {
        greeting = '어서 오시게. 무엇을 원하나?',
        menu = {
            { '물건 사기', function(ch, shop_npc)
                return npc.show_sell_menu(ch, shop_npc, ARMOR_SELL_CATEGORIES)
            end },
            { '물건 팔기', function(ch, shop_npc)
                return npc.show_buy_menu(ch, shop_npc)
            end },
            { '신수의옷 구입', function(ch, shop_npc)
                M.buy_clothes(ch, shop_npc)
                return DIALOG_RESULT.NEXT
            end },
        },
    })
end

function M.on_blacksmith_click(me, npc_obj)
    npc.shop(me, npc_obj, {
        greeting = '무엇을 도와 드릴까요?',
        menu = {
            { '물건 사기', function(ch, shop_npc)
                return npc.show_sell_menu(ch, shop_npc, WEAPON_SELL_CATEGORIES)
            end },
            { '물건 팔기', function(ch, shop_npc)
                return npc.show_buy_menu(ch, shop_npc)
            end },
            { '물건 고치기', function(ch, shop_npc)
                return npc.show_repair_menu(ch, shop_npc)
            end },
            { '모두 고치기', function(ch, shop_npc)
                return npc.show_repair_all_menu(ch, shop_npc)
            end },
            { '용무기 수리', function(ch, shop_npc)
                M.repair_dragon_weapon(ch, shop_npc)
                return DIALOG_RESULT.NEXT
            end },
        },
    })
end

function M.on_cloth_shop_click(me, npc_obj)
    npc.shop(me, npc_obj, {
        greeting = '어서 오시게.',
        menu = {
            { '물건 사기', function(ch, shop_npc)
                return npc.show_sell_menu(ch, shop_npc, CLOTH_SELL_CATEGORIES)
            end },
            { '물건 팔기', function(ch, shop_npc)
                return npc.show_buy_menu(ch, shop_npc)
            end },
        },
    })
end

function M.on_butcher_click(me, npc_obj)
    npc.shop(me, npc_obj, {
        greeting = '어서 오시게.',
        menu = {
            { '물건 사기', function(ch, shop_npc)
                return npc.show_sell_menu(ch, shop_npc, FOOD_SELL_CATEGORIES)
            end },
            { '물건 팔기', function(ch, shop_npc)
                return npc.show_buy_menu(ch, shop_npc)
            end },
        },
    })
end

function M.on_trainer_click(me, npc_obj)
    local totem = select(1, castle_lib.totem_of_map(me:map())) or '신수'
    local OPT_SINSOO = totem .. '마법진 배우기'
    local OPT_KUMGANG = totem .. '금강진 배우기'
    local OPT_BLESS = totem .. '축복진 배우기'
    local selected, button = me:pursuit(npc_obj, '수련은 멀고도 험한법...\n무엇을 원하시오?', { OPT_SINSOO, OPT_KUMGANG, OPT_BLESS })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if selected == OPT_KUMGANG or selected == OPT_BLESS then
        me:dialog(npc_obj, '......', { prev = false, next = false })
        return
    end

    if me:class() ~= CLASS.MAGE and me:class() ~= CLASS.POET then
        me:dialog(npc_obj, '문(文)의 길을 걷는 자만이 배울 수 있는 마법이라네...', { prev = false, next = false })
        return
    end

    local spell_name = totem .. '마법진'
    if me:dialog(npc_obj, '이 마법은 강력한 저주를 적에게 거는 마법일세. 이 마법을 시전하기 위해서는 특별한 대형을 유지해야만 하지. 또한 금전 50만전을 바쳐야 한다네.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
        return
    end

    local OPT_YES = '예. 배우겠습니다.'
    local OPT_NO = '아니오. 다음 기회에..'
    local confirm = me:pursuit(npc_obj, '마법을 배우겠는가?', { OPT_YES, OPT_NO })
    if confirm == OPT_NO then
        me:dialog(npc_obj, '준비가 되거든 언제든지 찾아오게...', { prev = false, next = false })
        return
    end
    if confirm ~= OPT_YES then
        return
    end
    if name2spell(spell_name) == nil then
        me:dialog(npc_obj, '......', { prev = false, next = false })
        return
    end
    if me:spell(spell_name) ~= nil then
        me:dialog(npc_obj, '이미 그 마법진을 알고 있군.', { prev = false, next = false })
        return
    end

    local price = 500000
    if me:money() < price then
        me:dialog(npc_obj, '금전이 부족하군...', { prev = false, next = false })
        return
    end
    if me:mkspell(spell_name) == nil then
        me:dialog(npc_obj, '더 이상 마법을 배울 수 없습니다.', { prev = false, next = false })
        return
    end
    me:money(me:money() - price)
    me:dialog(npc_obj, string.format('%s 가르쳐 주었네.', spell_name), { prev = false, next = false })
end

return M
