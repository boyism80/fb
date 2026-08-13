-- npc: 문파성선생

local castle_lib = require('lib.castle')

local DIVINE_BEAST_BY_CASTLE_MAP = {
    ['주작의눈'] = DIVINE_BEAST.VERMILION_BIRD,
    ['현무의눈'] = DIVINE_BEAST.BLACK_TORTOISE,
    ['청룡의눈'] = DIVINE_BEAST.AZURE_DRAGON,
    ['백호의눈'] = DIVINE_BEAST.WHITE_TIGER,
}

local function buy_clan_sword(me, npc)
    local button

    local clan = me:clan()
    if clan == nil then
        button = me:dialog(npc, '가입된 문파가 없습니다.', { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local is_master = false
    for _, member in pairs(clan:members()) do
        if member:role() == CLAN_ROLE.MASTER and member:name() == me:name() then
            is_master = true
            break
        end
    end
    if not is_master then
        button = me:dialog(npc, '너는 문파장이 아니구나!', { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local divine_beast = DIVINE_BEAST_BY_CASTLE_MAP[me:map():model():name()]
    if divine_beast == nil then
        return
    end

    local castle = castle(divine_beast)
    if castle == nil or castle:owner_clan() ~= clan:id() then
        button = me:dialog(npc, '이 성을 가진 문파장에게만 자격이 있다네.', { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local weapon_name = castle_lib.WEAPON_NAME_BY_DIVINE_BEAST[divine_beast]
    local has_weapon = #me:items(weapon_name) > 0
    if not has_weapon then
        local equipped = me:weapon()
        if equipped ~= nil and equipped:model():name() == weapon_name then
            has_weapon = true
        end
    end
    if has_weapon then
        button = me:dialog(npc, '자네는 이미 이 성을 대표하는 검을 가지고 있는 것 같군..', { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local model = name2item(weapon_name)
    if model == nil then
        return
    end
    local price = model:price()

    ::BUY_INTRO_1::
    button = me:dialog(npc, '이 성을 가진 주인이라면 마땅히 그 힘을 가질 자격이 있지.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::BUY_INTRO_2::
    button = me:dialog(npc, '그러나 그 힘을 얻기 위해서는 그에 마땅한 댓가로 따르는 법...', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BUY_INTRO_1
    end

    ::BUY_CONFIRM::
    local OPT_YES = '네. 지금 당장 주세요!'
    local OPT_NO = '아니요. 돈이 없네요...'
    local selected, list_btn = me:list(npc, string.format('그래, 금전 %d전을 댓가로 문파검을 구입하겠는가?', price), { OPT_YES, OPT_NO }, { prev = true })
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
        button = me:dialog(npc, '감히 어디서 거짓말인가!!', { prev = true, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto BUY_CONFIRM
        end
        return
    end

    if me:mkitem(weapon_name, 1) == nil then
        button = me:dialog(npc, '소지품이 가득 찼습니다.', { prev = true, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto BUY_CONFIRM
        end
        return
    end

    me:money(me:money() - price)

    ::BUY_DONE_1::
    button = me:dialog(npc, '이 검은 극도로 강한 기운을 잡아놓아 무척이나 불안하다네.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '죽으면 그 힘이 모두 사라지게 되니 조심해서 쓰시게나.', { prev = true, next = false })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BUY_DONE_1
    end
end

return {
    on_click = function(me, npc)
        local OPT_BUY = '문파검 구입'
        local selected, button = me:pursuit(npc, '성을 관리하기란 쉬운 일이 아니지. 그래, 무엇을 원하는가?', { OPT_BUY })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if selected ~= OPT_BUY then
            return
        end

        buy_clan_sword(me, npc)
    end
}
