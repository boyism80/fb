-- C++ entry points (see server/game/src/handler and object code).
local lib = require('lib.interaction')
local spell = require('lib.spell')
local npc = require('lib.npc')
local command = require('lib.command')
local castle_lib = require('lib.castle')

-- Gatekeeper NPC name -> { totem name, DIVINE_BEAST } for "~참가" chat.
local gatekeeper_by_name = {
    ["주작성문지기"] = { "주작", DIVINE_BEAST.VERMILION_BIRD },
    ["청룡성문지기"] = { "청룡", DIVINE_BEAST.AZURE_DRAGON },
    ["현무성문지기"] = { "현무", DIVINE_BEAST.BLACK_TORTOISE },
    ["백호성문지기"] = { "백호", DIVINE_BEAST.WHITE_TIGER },
}

local function run_black_flag(me, npc_obj)
    if me:dialog(npc_obj, '아니, 내가 검정깃발을 가지고 있다는걸 어떻게 알았나.. 으음...', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
        return true
    end
    if me:dialog(npc_obj, '그냥 줄순 없고.. 5000전만 내게. 그럼 검정깃발을 하나 주지.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
        return true
    end
    local selected, button = me:list(npc_obj, '어때? 5000전에 검정깃발 하나 사길 텐가?', { '네, 주십시오.', '안 살래요' })
    if button == DIALOG_RESULT.QUIT then
        return true
    end
    if selected ~= 1 then
        return true
    end
    local black_flag_price = 5000
    if me:money() < black_flag_price then
        me:dialog(npc_obj, '돈이 모자랍니다.', { prev = false, next = true })
        return true
    end
    if me:mkitem('검정깃발', 1) == nil then
        me:dialog(npc_obj, '공간이 부족합니다.', { prev = false, next = true })
        return true
    end
    me:money(me:money() - black_flag_price)
    me:dialog(npc_obj, '검정깃발을 받았습니다.', { prev = false, next = false })
    return true
end

local npc_chat_handlers = {
    {
        priority = 10,
        anchors = { '산다', '줘', '주세요' },
        pattern = lib.chat_regex.BUY,
        condition = function(npc_obj)
            return #npc_obj:model():sell() > 0
        end,
        func = function(me, npc_obj, params)
            local count = 1
            if params.count ~= nil then
                count = tonumber(params.count)
            end
            return npc.sell_item(me, npc_obj, params.name, count)
        end,
    },
    {
        priority = 20,
        anchors = { '판다', '팜', '팔게' },
        pattern = lib.chat_regex.SELL,
        condition = function(npc_obj)
            return npc_obj:model():buy() ~= nil
        end,
        func = function(me, npc_obj, params)
            local count = nil
            if params.all ~= nil then
                count = nil
            elseif params.count ~= nil then
                count = tonumber(params.count)
            else
                count = 1
            end
            return npc.buy_item(me, npc_obj, params.name, count)
        end,
    },
    {
        priority = 30,
        anchors = { '고쳐', '수리' },
        pattern = lib.chat_regex.REPAIR,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.REPAIR) == NPC_INTERACTION.REPAIR
        end,
        func = function(me, npc_obj, params)
            if params.all ~= nil then
                return npc.repair(me, npc_obj, nil)
            elseif params.name ~= nil then
                return npc.repair(me, npc_obj, params.name)
            else
                return false
            end
        end,
    },
    {
        priority = 40,
        anchors = { '맡아' },
        pattern = lib.chat_regex.DEPOSIT_MONEY,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end,
        func = function(me, npc_obj, params)
            local money = nil
            if params.all ~= nil then
                money = me:money()
            elseif params.money ~= nil then
                money = tonumber(params.money)
            else
                return false
            end
            return npc.deposit_money(me, npc_obj, money)
        end,
    },
    {
        priority = 50,
        anchors = { '돌려' },
        pattern = lib.chat_regex.WITHDRAW_MONEY,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end,
        func = function(me, npc_obj, params)
            local money = nil
            if params.all ~= nil then
                money = nil
            elseif params.money ~= nil then
                money = tonumber(params.money)
            else
                return false
            end
            return npc.withdraw_money(me, npc_obj, money)
        end,
    },
    {
        priority = 60,
        anchors = { '맡아' },
        pattern = lib.chat_regex.STORE_ITEM,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end,
        func = function(me, npc_obj, params)
            local count = nil
            if params.all ~= nil then
                count = nil
            elseif params.count ~= nil then
                count = tonumber(params.count)
            else
                count = 1
            end
            return npc.store_item(me, npc_obj, params.name, count)
        end,
    },
    {
        priority = 70,
        anchors = { '돌려' },
        pattern = lib.chat_regex.RETRIEVE_ITEM,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end,
        func = function(me, npc_obj, params)
            local count = nil
            if params.all ~= nil then
                count = nil
            elseif params.count ~= nil then
                count = tonumber(params.count)
            else
                count = 1
            end
            return npc.retrieve_item(me, npc_obj, params.name, count)
        end,
    },
    {
        priority = 80,
        anchors = { '파니', '파냐', '팔고' },
        pattern = lib.chat_regex.SELL_LIST,
        condition = function(npc_obj)
            return #npc_obj:model():sell() > 0
        end,
        func = function(me, npc_obj, params)
            return npc.sell_item_list(me, npc_obj)
        end,
    },
    {
        priority = 90,
        anchors = { '사니', '사냐', '사고' },
        pattern = lib.chat_regex.BUY_LIST,
        condition = function(npc_obj)
            return npc_obj:model():buy() ~= nil
        end,
        func = function(me, npc_obj, params)
            return npc.buy_item_list(me, npc_obj)
        end,
    },
    {
        priority = 100,
        anchors = { '얼마' },
        pattern = lib.chat_regex.SELL_PRICE,
        condition = function(npc_obj)
            return #npc_obj:model():sell() > 0
        end,
        func = function(me, npc_obj, params)
            return npc.sell_item_price(me, npc_obj, params.name)
        end,
    },
    {
        priority = 110,
        anchors = { '얼마에' },
        pattern = lib.chat_regex.BUY_PRICE,
        condition = function(npc_obj)
            return npc_obj:model():buy() ~= nil
        end,
        func = function(me, npc_obj, params)
            return npc.buy_item_price(me, npc_obj, params.name)
        end,
    },
    {
        priority = 120,
        anchors = { '맡고' },
        pattern = lib.chat_regex.DEPOSITED_MONEY,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end,
        func = function(me, npc_obj, params)
            return npc.deposited_money(me, npc_obj)
        end,
    },
    {
        priority = 130,
        anchors = { '명명', '이름을' },
        pattern = lib.chat_regex.RENAME_WEAPON,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.RENAME) == NPC_INTERACTION.RENAME
        end,
        func = function(me, npc_obj, params)
            return npc.rename_weapon(me, npc_obj, params.weapon, params.name)
        end,
    },
    {
        priority = 140,
        anchors = { '맡고' },
        pattern = lib.chat_regex.HOLD_ITEM_LIST,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end,
        func = function(me, npc_obj, params)
            return npc.store_item_list(me, npc_obj)
        end,
    },
    {
        priority = 150,
        anchors = { '맡고', '몇', '얼마나' },
        pattern = lib.chat_regex.HOLD_ITEM_COUNT,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end,
        func = function(me, npc_obj, params)
            return npc.store_item_count(me, npc_obj, params.name)
        end,
    },
    {
        priority = 160,
        anchors = { '살려' },
        pattern = lib.chat_regex.REVIVE,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.REVIVE) == NPC_INTERACTION.REVIVE
        end,
        func = function(me, npc_obj, params)
            return npc.revive(me, npc_obj, params.no ~= nil)
        end,
    },
    {
        priority = 170,
        anchors = { '감사합니다', '고맙습니다' },
        literal = true,
        condition = function(npc_obj)
            local interaction = npc_obj:model():interaction()
            return (interaction & NPC_INTERACTION.REVIVE) == NPC_INTERACTION.REVIVE
        end,
        func = function(me, npc_obj, params)
            return npc.appreciate(me, npc_obj)
        end,
    },
    {
        priority = 180,
        anchors = { '참가' },
        literal = true,
        condition = function(npc_obj)
            return gatekeeper_by_name[npc_obj:model():name()] ~= nil
        end,
        func = function(me, npc_obj, params)
            local name = npc_obj:model():name()
            local info = gatekeeper_by_name[name]
            return castle_lib.enter_castle(me, npc_obj, info[1], info[2])
        end,
    },
    {
        priority = 190,
        anchors = { '검정깃발' },
        literal = true,
        condition = function(npc_obj)
            return npc_obj:model():name() == '장안성대장간'
        end,
        func = function(me, npc_obj, params)
            return run_black_flag(me, npc_obj)
        end,
    },
    {
        priority = 200,
        anchors = { '안녕' },
        literal = true,
        condition = function(npc_obj)
            return npc_obj:model():name() == '용노인'
        end,
        func = function(me, npc_obj, params)
            me:script('scripts/npc/98.lua', 'on_hello', npc_obj)
            return true
        end,
    },
}

local npc_chat_handlers_by_anchor = {}

for _, handler in ipairs(npc_chat_handlers) do
    for _, anchor in ipairs(handler.anchors) do
        local bucket = npc_chat_handlers_by_anchor[anchor]
        if bucket == nil then
            bucket = {}
            npc_chat_handlers_by_anchor[anchor] = bucket
        end
        table.insert(bucket, handler)
    end
end

local function collect_npc_chat_handlers(message)
    local seen = {}
    local candidates = {}

    for anchor, handlers in pairs(npc_chat_handlers_by_anchor) do
        if string.find(message, anchor, 1, true) ~= nil then
            for _, handler in ipairs(handlers) do
                if seen[handler] == nil then
                    seen[handler] = true
                    table.insert(candidates, handler)
                end
            end
        end
    end

    table.sort(candidates, function(a, b)
        return a.priority < b.priority
    end)

    return candidates
end

local function dispatch_npc_chat(me, npcs, message)
    local candidates = collect_npc_chat_handlers(message)

    for _, handler in ipairs(candidates) do
        local params = nil
        if handler.literal then
            params = {}
        else
            params = regex(handler.pattern, message)
            if params == nil then
                goto continue_handler
            end
        end

        for _, npc_obj in ipairs(npcs) do
            if handler.condition ~= nil then
                if not handler.condition(npc_obj) then
                    goto continue_npc
                end
            end
            if handler.func(me, npc_obj, params) then
                return true
            end
            ::continue_npc::
        end
        ::continue_handler::
    end

    return false
end

local function make_baram_birth_label()
    local epoch = os.time{ year = 1996, month = 3, day = 31, hour = 0, min = 0, sec = 0 }
    local now = os.time()
    local diff_sec = now - epoch
    local total_ms = diff_sec * 1000

    -- 1 Baram day = 3 real hours = 10800000 milliseconds
    local baram_days = math.floor(total_ms / 10800000)
    local baram_year = math.floor(baram_days / 365) + 1
    local remain_days = baram_days % 365
    local baram_month = math.min(math.floor(remain_days / 30) + 1, 12)

    local season
    if baram_month == 12 or baram_month == 1 or baram_month == 2 then
        season = "겨울"
    elseif baram_month >= 3 and baram_month <= 5 then
        season = "봄"
    elseif baram_month >= 6 and baram_month <= 8 then
        season = "여름"
    else
        season = "가을"
    end

    return string.format("바람력 %d년 %s", baram_year, season)
end

-- Matchmaking events (see server/game/src/matchmaker.cpp, handler/amqp/matchmaking_*.cpp)
-- reason: 0=timeout, 1=decline
-- outcome: 0=excluded, 1=requeued
local matchmaking_states = {}

local function matchmaking_confirm_seconds(confirm_deadline)
    local y, mo, d, h, mi, se = confirm_deadline:match('^(%d+)-(%d+)-(%d+) (%d+):(%d+):(%d+)$')
    if y == nil then
        return 0
    end
    -- Parse absolute deadline string from matchmaking server (not for current time).
    local deadline = os.time({
        year = tonumber(y),
        month = tonumber(mo),
        day = tonumber(d),
        hour = tonumber(h),
        min = tonumber(mi),
        sec = tonumber(se),
    })
    return math.max(0, deadline - now())
end

local function matchmaking_clear_timer(me)
    me:timer(0, false)
end

local function matchmaking_state(me)
    local uid = me:uid()
    local state = matchmaking_states[uid]
    if state == nil then
        state = {}
        matchmaking_states[uid] = state
    end
    return state
end

local function matchmaking_clear_state(me)
    matchmaking_states[me:uid()] = nil
end

local function on_attack(me, additional_attack)
    local map = me:map()
    if map == nil then
        return 0
    end

    local pk = castle_lib.map_pk_enabled(me)
    local enemy_type = OBJECT_TYPE.LIFE
    if not pk or (me:is(OBJECT_TYPE.CHARACTER) and me:option(OPTION.PK_PROTECT)) then
        enemy_type = OBJECT_TYPE.MOB
    end

    if additional_attack == nil then
        additional_attack = false
    end

    if not additional_attack then
        me:action(ACTION.ATTACK, DURATION.ATTACK)
    end

    local weapon = nil
    local is_bow = false
    if me:is(OBJECT_TYPE.CHARACTER) and not additional_attack then
        weapon = me:weapon()
        if weapon ~= nil then
            local model = weapon:model()
            is_bow = model:type() == WEAPON_TYPE.BOW

            local sound = model:sound()
            if sound == 0 then
                if not is_bow then
                    sound = SOUND.SWING
                end
            end
            me:sound(sound)
        end
    end

    local count = 0
    local x, y = me:position()
    local direction = me:direction()
    if is_bow then
        local range = 14
        local target = nil
        local width = map:width()
        local height = map:height()
        for i = 1, range do
            local obj_x = x
            local obj_y = y
            if direction == DIRECTION.LEFT then
                obj_x = obj_x - i
            elseif direction == DIRECTION.RIGHT then
                obj_x = obj_x + i
            elseif direction == DIRECTION.TOP then
                obj_y = obj_y - i
            else
                obj_y = obj_y + i
            end

            if obj_x < 0 or obj_x > width or obj_y < 0 or obj_y > height then
                break
            end

            target = map:at(obj_x, obj_y, enemy_type)
            if target ~= nil then
                break
            end
        end

        if target ~= nil and not castle_lib.blocks_siege_friendly_fire(me, target) and not lib.is_miss(me, target, 'front') then
            lib.damage(me, target, nil, 701)
            count = count + 1
        end
    else
        local front = nil
        if me:is(OBJECT_TYPE.MOB) then
            front = me:target()
            if front ~= nil then
                local x_front, y_front = front:position()
                local x_diff = math.abs(x_front - x)
                local y_diff = math.abs(y_front - y)
                local is_near = (x_diff == 1 and y_diff == 0) or (y_diff == 1 and x_diff == 0)
                if not is_near then
                    front = nil
                end
            end
        end
        if front == nil then
            front = me:front(enemy_type)
        end
        local damaged_sound = nil
        if weapon ~= nil then
            damaged_sound = SOUND.DAMAGE
        end
        if front ~= nil and not castle_lib.blocks_siege_friendly_fire(me, front) and not lib.is_miss(me, front, 'front') then
            lib.damage(me, front, nil, damaged_sound)
            count = count + 1
        end

        if me:isbuff('측면공격') then
            local points = {}
            if direction == DIRECTION.LEFT or direction == DIRECTION.RIGHT then
                table.insert(points, {x, y-1})
                table.insert(points, {x, y+1})
            else
                table.insert(points, {x-1, y})
                table.insert(points, {x+1, y})
            end

            local nears = me:nears(enemy_type, points, false)
            for _, obj in pairs(nears) do
                if not castle_lib.blocks_siege_friendly_fire(me, obj) and not lib.is_miss(me, obj, 'side') then
                    lib.damage(me, obj, 0.4, damaged_sound)
                    count = count + 1
                end
            end
        end

        if me:isbuff('후면공격') then
            local points = {}
            if direction == DIRECTION.LEFT then
                table.insert(points, {x+1, y})
            elseif direction == DIRECTION.TOP then
                table.insert(points, {x, y+1})
            elseif direction == DIRECTION.RIGHT then
                table.insert(points, {x-1, y})
            else
                table.insert(points, {x, y-1})
            end

            local nears = me:nears(enemy_type, points, false)
            for _, obj in pairs(nears) do
                if not castle_lib.blocks_siege_friendly_fire(me, obj) and not lib.is_miss(me, obj, 'back') then
                    lib.damage(me, obj, 0.5, damaged_sound)
                    count = count + 1
                end
            end
        end
    end

    if me:isbuff('투명') and count > 0 then
        me:unbuff('투명')
    end

    if me:isbuff('분신') and not additional_attack then
        count = count + on_attack(me, true)
    end

    lib.any_action(me)
    return count
end

local function on_npc_chat(me, message, shout)
    local map = me:map()
    if map == nil then
        return false
    end

    local npcs = {}
    if shout then
        npcs = map:objects(OBJECT_TYPE.NPC)
    else
        local x, y = me:position()
        npcs = map:nears({x, y}, OBJECT_TYPE.NPC)
    end

    if #npcs == 0 then
        return false
    end

    return dispatch_npc_chat(me, npcs, message)
end

return {
    on_attack = on_attack,

    on_character_kill = function(killer, victim)
        if killer == nil or victim == nil then
            return
        end
        if not killer:is(OBJECT_TYPE.CHARACTER) or not victim:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not castle_lib.map_siege_castle(victim) then
            return
        end
        if not castle_lib.map_pk_enabled(victim) then
            return
        end

        broadcast(string.format('[정보] %s님이 %s님에게 죽어서 탈락했습니다.', victim:name(), killer:name()), MESSAGE_TYPE.WORLD)
    end,

    on_equipment_active = function(me, parts, equipment)
        lib.any_action(me)
    end,

    on_equipment_inactive = function(me, parts, equipment)
        if me:weapon_damage() > 0 then
            me:message('무기의 푸른빛이 사라집니다.')
            me:weapon_damage(0)
        end

        lib.any_action(me)
    end,

    on_loot = function(me)
        for _, buff_name in pairs(spell.relative_buff_name('투명')) do
            if me:isbuff(buff_name) then
                me:state(STATE.NORMAL)
                me:unbuff(buff_name)
            end
        end

        if me:super_hide() then
            me:super_hide(false)
            me:state(STATE.NORMAL)
        end

        lib.any_action(me)
    end,

    on_door = function(me)
        local map = me:map()
        local door = map:door(me)
        if door == nil then
            return
        end

        local key = me:item('파란열쇠')
        local locked = door:locked()
        if door:locked() and key == nil then
            me:message('문이 잠겼습니다.')
            return
        end

        local opened = door:toggle()
        if opened then
            me:message('문을 열었습니다.')
        else
            me:message('문을 닫았습니다.')
        end

        lib.any_action(me)
    end,

    on_move = function(me)
        lib.any_action(me)

        -- if me:is(OBJECT_TYPE.CHARACTER) then
        --     local quest = require('lib.quest')
        --     quest.red_clay_on_move(me)
        --     quest.mountain_treasure_fabric_on_move(me)
        -- end
    end,

    on_direction = function(me)
        lib.any_action(me)
    end,

    on_chat = function(me, message, shout)
        if string.sub(message, 1, 1) == '/' then
            message = string.sub(message, 2, string.len(message))
            args = lib.string_split(message, ' ')

            local cmd = args[1]
            if command.functions[cmd] == nil then
                return false
            end

            local cmd_data = command.functions[cmd]
            local cmd_func = nil
            local required_privilege = ROLE.USER

            if type(cmd_data) == 'table' then
                cmd_func = cmd_data['command']
                required_privilege = cmd_data['privilege'] or ROLE.USER
            else
                cmd_func = cmd_data
            end

            if me:role() < required_privilege then
                me:message('권한이 부족합니다.')
                return true
            end

            table.remove(args, 1)
            return cmd_func(me, args)
        end

        on_npc_chat(me, message, shout)
        return false
    end,

    on_npc_chat = on_npc_chat,

    on_clan_left = function(me)
        castle_lib.evict(me)
        castle_lib.strip_all(me)
    end,

    on_login = function(me, first_login)
        castle_lib.strip_all(me)
        castle_lib.evict(me)

        local clan = me:clan()
        if clan ~= nil and me:role() <= ROLE.USER then
            clan:message(string.format('[%s] %s님이 접속하셨습니다.', clan:name(), me:name()), MESSAGE_TYPE.NOTIFY)
        end

        if first_login then
            me:push_achievement(0, make_baram_birth_label() .. " 생", 0, 47)
        end
        if me:birthday() ~= nil then
            return
        end

        local npc = name2npc('낙랑')
        local button = nil
    ::BIRTHDAY_DIALOG_1::
        button = me:dialog(npc, '대단히 중요하니 끝까지 읽어주세요! 빈번히 발생하는 아이디 해킹을 미연에 방지하기 위해 또 하나의 2차 비밀번호를 정해야 합니다.', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

    ::BIRTHDAY_DIALOG_2::
        button = me:dialog(npc, '이 2차 비밀번호는 한번 결정을 하시면 변경이 불가능 합니다. 이 2차 비밀번호는 자신의 게임 접속 비밀번호를 바꿀 때에만 사용합니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto BIRTHDAY_DIALOG_1
        end

    ::BIRTHDAY_DIALOG_3::
        button = me:dialog(npc, '이 2차 비밀번호는 자신의 생년월일로 해주세요. 예를 들면 자신의 생일이 92년 04월 17일이면, 920417으로 입력해주세요. (반드시 6자리)', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto BIRTHDAY_DIALOG_2
        end

    ::BIRTHDAY_DIALOG_4::
        button = me:dialog(npc, '이 2차 비밀번호는 분실하실 이유가 없기 때문에(자신의 생년월일이므로) 이 2차 비밀번호를 모르시는 분은 해당 아이디에 대한 소유권을 박탈합니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto BIRTHDAY_DIALOG_3
        end

    ::BIRTHDAY_DIALOG_5::
        button = me:dialog(npc, '이 2차 비밀번호를 모르시고 처음의 비밀번호 분실신고를 하셔도 저희는 가르쳐 드릴수가 없습니다. 그 만큼 중요한 것이오니 신중히 그리고 정확히 입력해주세요.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto BIRTHDAY_DIALOG_4
        end

    ::BIRTHDAY_DIALOG_6::
        button = me:dialog(npc, '또한 이 두번째 비밀번호(생년월일)를 입력하지 않으시고 다음에 비밀번호 분실신고를 하셔도 저희는 해드릴수가 없으니 반드시 두번째 비번을 입력하시기 바랍니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto BIRTHDAY_DIALOG_5
        end

    ::BIRTHDAY_INPUT::
        local birthday = me:input(npc, '자신의 생년월일을 입력해 주세요.', { top = '저의 생년월일은', bottom = '입니다.', maxlen = 6, prev = true })
        if birthday == DIALOG_RESULT.QUIT then
            goto BIRTHDAY_INPUT
        end
        if birthday == DIALOG_RESULT.PREV then
            goto BIRTHDAY_DIALOG_6
        end

        ::BIRTHDAY_MUST_SET::
        if birthday == '' or #birthday ~= 6 then
            button = me:dialog(npc, '두번째 비밀번호를 정하지 않으면 게임을 할 수 없습니다. 반드시 해주세요.', { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto BIRTHDAY_INPUT
            end
            goto BIRTHDAY_INPUT
        end

        birthday = tonumber(birthday)
        if birthday == nil or birthday <= 0 then
            goto BIRTHDAY_MUST_SET
        end

        local sel, list_btn = me:list(npc, string.format('당신의 생년월일이 %s가 맞습니까?', birthday), {'예', '아니오'}, { prev = true })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto BIRTHDAY_INPUT
        end

        if sel == 1 then
            me:birthday(birthday)
            return
        end

        goto BIRTHDAY_MUST_SET
    end,

    on_matchmaking_register = function(me, match_type, registry_id)
        local state = matchmaking_state(me)
        local now_ts = now()
        state.queue_started_at = now_ts
        state.elapsed_seconds = 0
        me:timer(1200, false)
        me:message(string.format('매치메이킹 대기를 시작했습니다. (유형: %d)', match_type), MESSAGE_TYPE.STATE)
    end,

    on_matchmaking_unregister = function(me, match_type, registry_id)
        matchmaking_clear_timer(me)
        matchmaking_clear_state(me)
        me:message(string.format('매치메이킹 대기를 취소했습니다. (유형: %d)', match_type), MESSAGE_TYPE.STATE)
    end,

    on_matchmaking_decline = function(me, match_id, match_type)
        matchmaking_clear_timer(me)
        matchmaking_clear_state(me)
        me:message(string.format('매치를 거절했습니다. (유형: %d)', match_type), MESSAGE_TYPE.STATE)
    end,

    on_matchmaking_confirm = function(me, match_id, match_type)
        me:message(string.format('매치 참여를 수락했습니다. (유형: %d)', match_type), MESSAGE_TYPE.STATE)
    end,

    on_matchmaking_proposed = function(me, match_id, match_type, confirm_deadline)
        local state = matchmaking_state(me)
        local now_ts = now()
        if state.queue_started_at ~= nil then
            state.elapsed_seconds = now_ts - state.queue_started_at
        else
            state.elapsed_seconds = 0
        end
        local confirm_seconds = matchmaking_confirm_seconds(confirm_deadline)
        me:timer(confirm_seconds, true)
        me:message(
            string.format('매치가 제안되었습니다. %d초 안에 수락해 주세요. (유형: %d)', confirm_seconds, match_type),
            MESSAGE_TYPE.STATE
        )

        local npc = name2npc('낙랑')
        local selected, button = me:list(npc, '매치를 찾았습니다. 참여하시겠습니까?', {'예', '아니오'}, { prev = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if selected == 1 then
            local err = me:matchmaker():confirm()
            if err ~= nil then
                me:message(err, MESSAGE_TYPE.STATE)
            end
        else
            local err = me:matchmaker():decline()
            if err ~= nil then
                me:message(err, MESSAGE_TYPE.STATE)
            end
        end
    end,

    on_matchmaking_ready = function(me, match_id, match_type)
        matchmaking_clear_timer(me)
        matchmaking_clear_state(me)
        me:message(string.format('매치가 성사되었습니다. (유형: %d)', match_type), MESSAGE_TYPE.STATE)
    end,

    on_matchmaking_dissolved = function(me, match_id, match_type, reason, outcome)
        if outcome == 1 then
            local state = matchmaking_state(me)
            local elapsed = state.elapsed_seconds or 0
            state.queue_started_at = now() - elapsed
            me:timer(1200, false)
            if reason == 0 then
                me:message(
                    string.format('상대가 응답하지 않아 대기를 이어갑니다. (유형: %d)', match_type),
                    MESSAGE_TYPE.STATE
                )
            else
                me:message(
                    string.format('다른 플레이어가 거절하여 대기를 이어갑니다. (유형: %d)', match_type),
                    MESSAGE_TYPE.STATE
                )
            end
        else
            local had_state = matchmaking_states[me:uid()] ~= nil
            matchmaking_clear_timer(me)
            matchmaking_clear_state(me)
            if not had_state then
                return
            end
            if reason == 0 then
                me:message(string.format('응답 시간이 초과되어 대기가 종료되었습니다. (유형: %d)', match_type), MESSAGE_TYPE.STATE)
            else
                me:message(string.format('매치가 취소되었습니다. (유형: %d)', match_type), MESSAGE_TYPE.STATE)
            end
        end
    end
}
