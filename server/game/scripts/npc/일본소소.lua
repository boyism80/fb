local HAIR_STYLE_PRICE = 100000
local HAIR_STYLE_MIN = 0
local HAIR_STYLE_MAX = 98

local DYE_OPTIONS = {
    { '원래 색깔로 돌려주세요.', 100 },
    { '녹색', 100 },
    { '감청색', 100 },
    { '회갈색', 100 },
    { '적갈색', 100 },
    { '황토색', 100 },
    { '연보라색', 100 },
    { '보라색', 100 },
    { '바다색', 100 },
    { '흰색(500전)', 500 },
    { '귤색(500전)', 500 },
    { '짙은곤색(500전)', 500 },
    { '회색(500전)', 500 },
    { '오렌지색(500전)', 500 },
    { '갈색(500전)', 500 },
    { '연갈색(500전)', 500 },
    { '짙은회색(500전)', 500 },
    { '밝은회색(500전)', 500 },
}

local function main_menu(me, npc)
    local index = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {'머리모양을 바꾸고 싶어요','머리염색'}, false)
    if index == nil then
        return
    end
    if index == 1 then
        do_hair_style(me, npc)
    elseif index == 2 then
        do_hair_dye(me, npc)
    end
end

function do_hair_style(me, npc)
    local index = me:list(npc, '가격에 따라 다른 머리모양을 해드릴 수 있습니다.\n\n원하시는 가격을 선택해주세요.', {'10만전'}, false)
    if index == nil or index ~= 1 then
        return
    end
    
    local face = HAIR_STYLE_MIN
    local hair_color = me:color()
    
    while true do
        local appearance = {
            ['hair'] = face,
            ['hair_color'] = hair_color,
        }
        local msg = '이 모습으로 바꾸시겠습니까?\n\n비용은 10만전 입니다.'
        local sel = me:list(appearance, msg, {'이 모습을 선택하겠습니다.', '다른 머리모양을 보여주세요.'}, false)
        if sel == nil then
            return
        end
        
        if sel == 1 then
            local money = me:money()
            if money < HAIR_STYLE_PRICE then
                me:dialog(npc, '금액이 ' .. (HAIR_STYLE_PRICE - money) .. '전 부족합니다.', false, false)
                return
            end
            me:look(face)
            me:money(money - HAIR_STYLE_PRICE)
            me:dialog(npc, '선택하신 머리로 설정해 드렸습니다.', false, false)
            return
        end
        
        if sel == 2 then
            face = face + 1
            if face <= HAIR_STYLE_MAX then
            else
                local again = me:list(npc, '방금 보신 것이 마지막입니다.\n\n처음부터 다시 보시겠습니까?', {
                    '예',
                    '아니오',
                }, false)
                if again == nil then
                    return
                end
                if again == 1 then
                    face = HAIR_STYLE_MIN
                else
                    return
                end
            end
        end
    end
end

function do_hair_dye(me, npc)
    local btn = me:dialog(npc, '염색을 하시려구요?\n염색 비용은 100전 이고, 이번에 새로 가져온 염색약은 비싸서 500전 이에요.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    
    local dye_menu = {}
    for i, opt in ipairs(DYE_OPTIONS) do
        dye_menu[i] = opt[1]
    end
    local choice = me:list(npc, '원하는 염색을 선택해라', dye_menu, false)
    if choice == nil or choice < 1 or choice >= #DYE_OPTIONS then
        return
    end
    
    local color_index = choice
    local cost = DYE_OPTIONS[choice][2]
    local money = me:money()
    if money < cost then
        me:dialog(npc, '금액이 ' .. (cost - money) .. '전 부족합니다.', false, false)
        return
    end
    
    me:color(color_index)
    me:money(money - cost)
    me:dialog(npc, '예쁘게 염색이 되었습니다~', false, false)
end

function NPC_JapanSoso(me, npc)
    main_menu(me, npc)
end