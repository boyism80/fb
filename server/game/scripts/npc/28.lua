-- npc: 도호귀인
local HP_MIN_LEVEL = 90

local HP_PROMOTION_CAP = {
    warrior = { 80000, 250000, 700000, 1600000 },
    mage    = { 35000, 170000, 550000, 1000000 },
}

local HP_CLASS_PROFILE = {
    [CLASS.WARRIOR] = 'warrior',
    [CLASS.ROGUE]   = 'warrior',
    [CLASS.MAGE]    = 'mage',
    [CLASS.POET]    = 'mage',
}

local HP_TIERS = {
    warrior = {
        { min_hp = 0,       exp = 10000000,   hp = 50 },
        { min_hp = 800000,  exp = 20000000,   hp = 50 },
        { min_hp = 1000000, exp = 50000000,   hp = 50 },
        { min_hp = 1200000, exp = 100000000,  hp = 50 },
        { min_hp = 1500000, exp = 200000000,  hp = 50 },
        { min_hp = 1700000, exp = 300000000,  hp = 50 },
        { min_hp = 2000000, exp = 400000000,  hp = 50 },
        { min_hp = 2200000, exp = 600000000,  hp = 50 },
        { min_hp = 2500000, exp = 900000000,  hp = 50 },
        { min_hp = 3000000, exp = 2000000000, hp = 50 },
        { min_hp = 3100000, exp = 2000000000, hp = 25 },
        { min_hp = 3200000, exp = 2000000000, hp = 10 },
        { min_hp = 3300000, exp = 2000000000, hp = 5 },
        { min_hp = 3400000, exp = 2000000000, hp = 3 },
        { min_hp = 3500000, exp = 2000000000, hp = 1 },
    },
    mage = {
        { min_hp = 0,       exp = 10000000,   hp = 50 },
        { min_hp = 400000,  exp = 20000000,   hp = 50 },
        { min_hp = 500000,  exp = 50000000,   hp = 50 },
        { min_hp = 600000,  exp = 100000000,  hp = 50 },
        { min_hp = 700000,  exp = 200000000,  hp = 50 },
        { min_hp = 800000,  exp = 300000000,  hp = 50 },
        { min_hp = 900000,  exp = 400000000,  hp = 50 },
        { min_hp = 1000000, exp = 600000000,  hp = 50 },
        { min_hp = 1100000, exp = 900000000,  hp = 50 },
        { min_hp = 1200000, exp = 1200000000, hp = 50 },
        { min_hp = 1250000, exp = 2000000000, hp = 50 },
        { min_hp = 1300000, exp = 2000000000, hp = 25 },
        { min_hp = 1400000, exp = 2000000000, hp = 10 },
        { min_hp = 1500000, exp = 2000000000, hp = 5 },
    },
}

local MP_PROMOTION_CAP = {
    warrior = nil,
    mage    = { 30000, 110000, 550000, 1000000 },
}

local MP_TIERS = {
    warrior = {
        { min_mp = 0,      exp = 10000000,   mp = 25 },
        { min_mp = 100000, exp = 40000000,   mp = 25 },
        { min_mp = 150000, exp = 50000000,   mp = 25 },
        { min_mp = 200000, exp = 100000000,  mp = 25 },
        { min_mp = 250000, exp = 200000000,  mp = 25 },
        { min_mp = 300000, exp = 200000000,  mp = 25 },
        { min_mp = 400000, exp = 400000000,  mp = 25 },
        { min_mp = 450000, exp = 600000000,  mp = 25 },
        { min_mp = 500000, exp = 1000000000, mp = 25 },
        { min_mp = 550000, exp = 2000000000, mp = 25 },
        { min_mp = 600000, exp = 2000000000, mp = 15 },
        { min_mp = 650000, exp = 2000000000, mp = 10 },
        { min_mp = 700000, exp = 2000000000, mp = 5 },
    },
    mage = {
        { min_mp = 0,       exp = 10000000,   mp = 25 },
        { min_mp = 600000,  exp = 20000000,   mp = 25 },
        { min_mp = 700000,  exp = 50000000,   mp = 25 },
        { min_mp = 800000,  exp = 100000000,  mp = 25 },
        { min_mp = 900000,  exp = 200000000,  mp = 25 },
        { min_mp = 1000000, exp = 300000000,  mp = 25 },
        { min_mp = 1100000, exp = 400000000,  mp = 25 },
        { min_mp = 1200000, exp = 600000000,  mp = 25 },
        { min_mp = 1400000, exp = 900000000,  mp = 25 },
        { min_mp = 1550000, exp = 2000000000, mp = 25 },
        { min_mp = 1600000, exp = 2000000000, mp = 15 },
        { min_mp = 1650000, exp = 2000000000, mp = 10 },
        { min_mp = 1700000, exp = 2000000000, mp = 5 },
        { min_mp = 1750000, exp = 2000000000, mp = 3 },
        { min_mp = 1800000, exp = 2000000000, mp = 1 },
    },
}

local function hp_tier_for(profile, base_hp)
    local tiers = HP_TIERS[profile]
    for i = #tiers, 1, -1 do
        if base_hp >= tiers[i].min_hp then
            return tiers[i]
        end
    end
    return tiers[1]
end

local function mp_tier_for(profile, base_mp)
    local tiers = MP_TIERS[profile]
    for i = #tiers, 1, -1 do
        if base_mp >= tiers[i].min_mp then
            return tiers[i]
        end
    end
    return tiers[1]
end

local STAT_MIN_LEVEL = 99
local STAT_EXP_COST = 10000000
local STAT_CAP = 130

local function run_buy_stat(me, npc, stat_key, stat_name)
    if me:level() < STAT_MIN_LEVEL then
        if me:dialog(npc, '99레벨 이상만 가능하네.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
            return false
        end
        return true
    end
    local getter = ({ str = me.base_str, dex = me.base_dex, int = me.base_int })[stat_key]
    local setter = ({ str = me.base_str, dex = me.base_dex, int = me.base_int })[stat_key]
    local current = getter(me)
    if current >= STAT_CAP then
        if me:dialog(npc, '더 이상 능력치를 올리실 수 없습니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
            return false
        end
        return true
    end
    local my_exp = me:exp()
    if my_exp < STAT_EXP_COST then
        if me:dialog(npc, '경험치가 부족하네.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
            return false
        end
        return true
    end
    setter(me, current + 1)
    me:exp(my_exp - STAT_EXP_COST)
    if me:dialog(npc, string.format('정상적으로 %s 1 올려주었네.', stat_name), { prev = false, next = true }) == DIALOG_RESULT.QUIT then
        return false
    end
    return true
end

local function run_init_dialogs(me, npc)
    local button
    ::NPC_28_0001::
    button = me:dialog(npc, '나에게 또 무슨 도움을 받겠다고 찾아왔느냐...', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_28_0002::
    button = me:dialog(npc, '여기 저기서 사고나 치고 돌아다니면서, 툭하면 나를 찾아오는 속셈이 도대체 무엇이냐?', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_28_0001
    end
    ::NPC_28_0003::
    button = me:dialog(npc, '그리고 나를 찾아올 생각을 했다면 뭔가를 들고 왔어야지... 만약 뭔가를 공짜로 얻으려고 생각했다면 큰 오산이니 바로 돌아가거라.', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_28_0002
    end
    return true
end

local function run_buy_hp(me, npc)
    local button

    button = me:dialog(npc, '체력을 사기 전에 명심하게.. 지금 착용하고 있는 모든 아이템을 벗은 후, 체력을 사기 바라네.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    while true do
        button = me:dialog(npc, '만약, 내 말을 듣지 않고, 체력을 사서 일어나는 불이익에 대해서는 절대 책임을 질 수 없네!!', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end

    while true do
        button = me:dialog(npc, '경험치로 체력을 사고 싶다고.... 늘 이렇게 경험치를 팔고는 여기 저기 가서 잘못 팔았다고 후회를 할 것이거늘... 쯧쯧... 어디 레벨이나 되는지 한 번 보자.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end

    if me:level() < HP_MIN_LEVEL then
        me:dialog(npc, '수련을 더 쌓으시게.', { prev = false, next = true })
        return true
    end

    button = me:dialog(npc, '레벨은 충분하고...', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    if button == DIALOG_RESULT.PREV then
        return true
    end

    local profile = HP_CLASS_PROFILE[me:class()] or 'warrior'
    local promotion = me:promotion()
    local base_hp = me:base_hp()
    local caps = HP_PROMOTION_CAP[profile]
    if caps and caps[promotion] and base_hp >= caps[promotion] then
        me:dialog(npc, '아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!', { prev = false, next = true })
        return true
    end

    local tier = hp_tier_for(profile, base_hp)
    local step = me:input(npc, string.format('그래 체력을 몇 단계나 올리려고? 체력은 한 단계가 %d이라는 것은 알고 있겠지?', tier.hp), { top = '예,', bottom = '단계를 올리고 싶습니다.' })
    if step == nil then
        return false
    end
    local step_num = tonumber(step) or 0
    local my_exp = me:exp()
    local exp_cost = step_num * tier.exp
    if my_exp < exp_cost then
        me:dialog(npc, '경험치가 부족하네', { prev = false, next = true })
        return true
    end

    me:exp(my_exp - exp_cost)
    me:base_hp(me:base_hp() + (step_num * tier.hp))

    button = me:dialog(npc, string.format('자네의 경험치로 체력을 %d단계 올렸으니 그리 알고 돌아가도록 해.', step_num), { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    while true do
        button = me:dialog(npc, string.format('화면에 바로 보일런지는 모르겠지만, 자네의 체력은 이제 %d이네', me:maxhp()), { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end
    button = me:dialog(npc, '강인한 체력을 나쁜 곳에 쓰려고 한다면... 내가 자네를 가만두지 않겠네. 알겠나?', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    return true
end

local function run_buy_mp(me, npc)
    local button

    button = me:dialog(npc, '마력을 사기 전에 명심하게.. 지금 착용하고 있는 모든 아이템을 벗은 후, 마력을 사기 바라네.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    while true do
        button = me:dialog(npc, '만약, 내 말을 듣지 않고, 마력을 사서 일어나는 불이익에 대해서는 절대 책임을 질 수 없네!!', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end

    while true do
        button = me:dialog(npc, '경험치로 마력을 사고 싶다고.... 늘 이렇게 경험치를 팔고는 여기 저기 가서 잘못 팔았다고 후회를 할 것이거늘... 쯧쯧... 어디 레벨이나 되는지 한 번 보자.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end

    if me:level() < HP_MIN_LEVEL then
        me:dialog(npc, '수련을 더 쌓으시게.', { prev = false, next = true })
        return true
    end

    button = me:dialog(npc, '레벨은 충분하고...', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    if button == DIALOG_RESULT.PREV then
        return true
    end

    local profile = HP_CLASS_PROFILE[me:class()] or 'warrior'
    local promotion = me:promotion()
    local base_mp = me:base_mp()
    local caps = MP_PROMOTION_CAP[profile]
    if caps and caps[promotion] and base_mp >= caps[promotion] then
        me:dialog(npc, '더 이상 올릴 수 없습니다.', { prev = false, next = true })
        return true
    end

    local tier = mp_tier_for(profile, base_mp)
    local step = me:input(npc, string.format('그래 마력을 몇 단계나 올리려고? 마력은 한 단계가 %d이라는 것은 알고 있겠지?', tier.mp), { top = '예,', bottom = '단계를 올리고 싶습니다.' })
    if step == nil then
        return false
    end
    local step_num = tonumber(step) or 0
    local my_exp = me:exp()
    local exp_cost = step_num * tier.exp
    if my_exp < exp_cost then
        me:dialog(npc, '경험치가 부족하네', { prev = false, next = true })
        return true
    end

    me:exp(my_exp - exp_cost)
    me:base_mp(me:base_mp() + (step_num * tier.mp))

    button = me:dialog(npc, string.format('자네의 경험치로 마력을 %d단계 올렸으니 그리 알고 돌아가도록 해.', step_num), { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    while true do
        button = me:dialog(npc, string.format('화면에 바로 보일런지는 모르겠지만, 자네의 마력은 이제 %d이네', me:maxmp()), { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end
    button = me:dialog(npc, '강인한 마력을 나쁜 곳에 쓰려고 한다면... 내가 자네를 가만두지 않겠네. 알겠나?', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    return true
end

local function run_change_face(me, npc)
    local index, button = me:list(npc, '지금의 모습을 벗어나 새로운 얼굴을 갖기를 원하고 있나?', {'예', '아니오'})
    if index ~= 1 then
        return false
    end
    index, button = me:list(npc, '사회에서 물의를 빚고 숨어서 지내려는 것은 아니겠지?', {'아닙니다.', '예. 잠시 숨어서....'})
    if index ~= 1 then
        return false
    end
    index, button = me:list(npc, '수술을 하려면 말일세, 백만전의 금이 필요하다네.. 가지고 있는가?', {'예', '아니오'})
    if index ~= 1 then
        return false
    end

    while true do
        button = me:dialog(npc, '지금부터 차례로 자네가 갖을 수 있는 모습을 보여주도록 하겠다. 수술이 그리 쉽지는 않고', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        button = me:dialog(npc, '원하는 모습이 보이면 바로 예를 선택하도록. 현재 네가 가질 수 있는 얼굴은 101개가 있다. 이전이나 다음을 선택해도 얼굴이 바뀌지 않으면 마지막 모습이니 그리 알도록.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end

    local hair = 0
    while true do
        local portrait = {
            ['hair'] = hair,
            ['hair_color'] = me:color(),
            ['gender'] = me:gender()
        }
        local equipments = me:equipments()
        if equipments[EQUIPMENT_PARTS.ARMOR] ~= nil then
            local model = equipments[EQUIPMENT_PARTS.ARMOR]:model()
            portrait['armor'] = model:dress()
            portrait['armor_color'] = me:armor_color()
        end
        if equipments[EQUIPMENT_PARTS.WEAPON] ~= nil then
            local model = equipments[EQUIPMENT_PARTS.WEAPON]:model()
            portrait['weapon'] = model:dress()
        end
        if equipments[EQUIPMENT_PARTS.SHIELD] ~= nil then
            local model = equipments[EQUIPMENT_PARTS.SHIELD]:model()
            portrait['shield'] = model:dress()
        end
        index, button = me:list(portrait, '다음에 또 수술을 해 줄 수는 있지만, 수술을 계속한다고 결코 좋아지지는 않는다. 이 모습을 선택하겠느냐?', {'선택', '이전 모습으로', '다음 모습으로', '수술 포기'}, { prev = false })
        if index == nil then
            return false
        end

        if index == 1 then
            if me:dialog(npc, '그럼 네 얼굴을 그 모습을 고쳐주도록 하지.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return false
            end
            local money = me:money()
            if money < 1000000 then
                if me:dialog(npc, '백만원이 없잖아 임마', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                    return false
                end
                return true
            end
            me:hair(hair)
            me:money(money - 1000000)
            if me:dialog(npc, '새 얼굴이 마음에 드는가? 맘에 안들어도 다시 해 줄 수는 없네.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return false
            end
            if me:dialog(npc, '나에게 더 도움을 받을 일은 이제 없겠지?.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return false
            end
            return true
        elseif index == 2 then
            hair = (hair - 1 + 102) % 102
        elseif index == 3 then
            hair = (hair + 1) % 102
        elseif index == 4 then
            me:dialog(npc, '수술 포기')
            return false
        end
    end
end

local function run_change_gender(me, npc)
    local button
    while true do
        button = me:dialog(npc, '이건 아직 해 줄 수가 없네...', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        button = me:dialog(npc, '나도 조금 더 공부를 해야 하고... 그리고 하여간 여러가지 복잡한 사정이 있으니... 이번ㅂ에는 그냥 돌아가도록 하게.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button ~= DIALOG_RESULT.PREV then
            break
        end
    end

    local index, list_btn = me:list(npc, '아니 돌아가라고 하는데, 왜 이렇게 나를 귀찮게 하지? 이것을 꼭 해야 할 사정이 있나? 비용도 만만치 않게 들고, 그리고 실패할 확률도 많은데, 그래도 꼭 해야 되겠나?', {'예', '아니오. 그럼 나중에...'})
    if list_btn == DIALOG_RESULT.QUIT then
        return false
    end
    if index ~= 1 then
        return false
    end

    button = me:dialog(npc, '정 그렇다면... 먼저 아이템을 장비하고 있는지 보겠네. 아이템을 장비한 채로 성전환을 하면 부작용이 있을 수도 있지.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    local equipments = me:equipments()
    local naked = true
    for _parts, _equipment in pairs(equipments) do
        naked = false
        break
    end
    if not naked then
        me:dialog(npc, '장비를 착용중이라서 불가능')
        return false
    end

    button = me:dialog(npc, '아이템은 장비하지 않았고... 필요한 수술비를 가지고 왔는지 좀 보겠네.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    local money = me:money()
    local price = 100000
    if price > money then
        me:dialog(npc, '돈이 모자람')
        return false
    end

    button = me:dialog(npc, '성을 전환하려면 십만전이 필요하다네... 금전을 지불하면 돌려주지 않을 것이니 지금 신중하게 생각하시게. 왜냐하면... 하여간 그렇다네. 그 정도는 준비해 왔겠지?', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    button = me:dialog(npc, '먼저 수술비를 받았으니... 이제 자네를 조금 검사해 보아야 되겠네.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end

    local m = me:marriage()
    if m.married then
        me:dialog(npc, '음... 결혼한 상태에서는 성전환 수술을 할 수 없네. 먼저 이혼을 한 뒤에 다시 오도록 하게.')
        return false
    end

    button = me:dialog(npc, '음... 아직 결혼한 상태가 아니니... 수술을 시작해 봐야지.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end

    local gender = me:gender()
    local gender_from, gender_to
    if gender == GENDER.MALE then
        gender_from = '남자'
        gender_to = '여자'
    else
        gender_from = '여자'
        gender_to = '남자'
    end
    index, list_btn = me:list(npc, string.format('자네 %s가 되고 싶다는 것이지?', gender_to), {'예', '아니오'})
    if list_btn == DIALOG_RESULT.QUIT then
        return false
    end
    if index ~= 1 then
        return false
    end

    button = me:dialog(npc, '그래. 내 수술비도 받고, 자네가 결혼한 몸도 아니라는 것을 확인했으니, 성별을 바꾸어는 주겠네. 하지만 이런 일을 다시는 내게 와서 부탁하지 말게. 알겠나?', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end

    me:money(money - price)
    if gender == GENDER.MALE then
        me:gender(GENDER.FEMALE)
    else
        me:gender(GENDER.MALE)
    end
    button = me:dialog(npc, string.format('자네 지금부터는 %s 되었네. %s용 의복을 갖추고 있는지는 몰라도, 하여간 자네는 이제 %s의 몸을 갖게 되었으니 그렇게 알고 돌아고도록 하게.', gender_to, gender_from, gender_to), { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    return true
end

return {
    on_click = function(me, npc)
        while true do
            if not run_init_dialogs(me, npc) then
                return
            end
            local index, button = me:pursuit(npc, '그래도 버티고 서서 도대체 원하는 것이 뭐냐?', {
                '힘올리기', '지력올리기', '민첩올리기', '체력사기', '마력사기', '성형', '성전환',
            })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if index == 1 then
                if not run_buy_stat(me, npc, 'str', '힘') then
                    return
                end
            elseif index == 2 then
                if not run_buy_stat(me, npc, 'int', '지력') then
                    return
                end
            elseif index == 3 then
                if not run_buy_stat(me, npc, 'dex', '민첩성') then
                    return
                end
            elseif index == 4 then
                if not run_buy_hp(me, npc) then
                    return
                end
            elseif index == 5 then
                if not run_buy_mp(me, npc) then
                    return
                end
            elseif index == 6 then
                if not run_change_face(me, npc) then
                    return
                end
            elseif index == 7 then
                if not run_change_gender(me, npc) then
                    return
                end
            else
                return
            end
        end
    end
}
