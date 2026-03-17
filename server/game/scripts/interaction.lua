CHAT_REGEX = {
    SELL = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개)|(?P<all>다|전부))\\s+)?(?:판다|팜|팔게)",
    BUY = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개))\\s+)?(?:산다|줘|주세요)",
    REPAIR = "(((?P<all>전부|모두|다)|(?P<name>\\S+))\\s+?(?:고쳐|수리\\s*해))\\s*줘",
    DEPOSIT_MONEY = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+맡아\\s*(?:줘|놔|주세요)",
    WITHDRAW_MONEY = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+돌려\\s*(?:줘|놔|주세요)",
    STORE_ITEM = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?맡아\\s*(?:줘|놔|주세요)",
    RETRIEVE_ITEM = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?돌려\\s*(?:줘|놔|주세요)",
    SELL_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*(?:파니|파냐|팔고\\s*(?:있니|있냐))",
    BUY_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*(?:사니|사냐|사고\\s*(?:있니|있냐))",
    SELL_PRICE = "(?P<name>\\S+)\\s+얼마(?:(?:(?:니|야|임|냐|에\\s*파(?:니|냐)))|(?:파(?:니|냐|)))",
    BUY_PRICE = "(?P<name>\\S+)\\s+얼마에\\s?사(?:니|냐)",
    DEPOSITED_MONEY = "(?:돈|금전)\\s*얼마(?:나)?\\s*맡(?:아두)?고\\s*있(?:니|냐)",
    RENAME_WEAPON = "(?P<weapon>\\S+?)?(?:의|$)?\\s+이름을\\s+(?P<name>\\S+?)?(?:으|$)?로\\s+명명",
    HOLD_ITEM_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*맡고\\s*(?:있니|있냐)",
    HOLD_ITEM_COUNT = "(?P<name>\\S+)\\s+(?:몇\\s*개|얼마나)\\s*맡고\\s*있(?:니|냐)",
    REVIVE = "살려(?:(?P<ok>(?:주세요|주십시오))|(?P<no>(?:줘|내|라|주소)))",
    APPRECIATE = "(감사합니다|고맙습니다)",
    JOIN_CASTLE = "참가",
    BLACK_FLAG = "검정깃발",
}

function string_split(self, delimiter)
    local result = { }
    local from  = 1
    local delim_from, delim_to = string.find( self, delimiter, from  )
    while delim_from do
        table.insert( result, string.sub( self, from , delim_from-1 ) )
        from  = delim_to + 1
        delim_from, delim_to = string.find( self, delimiter, from  )
    end
    table.insert( result, string.sub( self, from  ) )
    return result
end

function is_miss(me, you)
    if debug() then
        return false
    else
        return math.random() > 0.8
    end
end

function is_critical(me, you)
    if debug() then
        return true
    else
        return math.random() > 0.8
    end
end

function damage(me, you, rate, sound)
    if rate == nil then
        rate = 1.0
    end
    rate = rate * (me:damage_rate() / 1000.0)

    local size = MOB_SIZE.SMALL
    if you:is(OBJECT_TYPE.MOB) then
        local model = you:model()
        size = model:size()
    end

    if me:isbuff('투명') then
        rate = rate * 8
    end

    local critical = is_critical()
    if sound ~= nil then
        you:sound(sound)
    end
    you:damage(me:normal_attack_damage(size), me, { critical = critical, rate = rate })
end

function on_attack(me, additional_attack)
    local map = me:map()
    if map == nil then
        return 0
    end

    local option = map:model():option()
    local pk = (option & MAP_OPTION.ENABLE_PK) == MAP_OPTION.ENABLE_PK
    local enemy_type = OBJECT_TYPE.LIFE
    if not pk then
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

        if target ~= nil then
            damage(me, target, nil, 701)
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
        if front ~= nil and not is_miss(me, front) then
            damage(me, front, nil, damaged_sound)
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
                damage(me, obj, 0.4, damaged_sound)
                count = count + 1
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
                damage(me, obj, 0.5, damaged_sound)
                count = count + 1
            end
        end
    end

    if me:isbuff('투명') and count > 0 then
        me:unbuff('투명')
    end

    if me:isbuff('분신') and not additional_attack then
        count = count + on_attack(me, true)
    end

    any_action(me)
    return count
end

function on_equipment_active(me, parts, equipment)
    any_action(me)
end

function on_equipment_inactive(me, parts, equipment)
    if me:weapon_damage() > 0 then
        me:message('무기의 푸른빛이 사라집니다.')
        me:weapon_damage(0)
    end

    any_action(me)
end

function on_loot(me)
    for _, buff_name in pairs(relative_buff_name('투명')) do
        if me:isbuff(buff_name) then
            me:state(STATE.NORMAL)
            me:unbuff(buff_name)
        end
    end

    if me:super_hide() then
        me:super_hide(false)
        me:state(STATE.NORMAL)
    end

    any_action(me)
end

function on_door(me)
    local map = me:map()
    local door = map:door(me)
    if door == nil then
        return
    end

    local key = me:item('파란열쇠')
    local locked = door:locked()
    if door:locked() and key == nil then
        me:message('문이 잠겨있습니다.')
        return
    end

    local opened = door:toggle()
    if opened then
        me:message('문을 열었습니다.')
    else
        me:message('문을 닫았습니다.')
    end

    any_action(me)
end

function red_clay_on_move(me)
    local RED_CLAY_ACHIEVEMENT_ID = 22
    local RED_CLAY_PARENT_MAP_NAMES = { ['고균도'] = true, ['가릉도'] = true, ['폭염도'] = true }
    local RED_CLAY_MAX_PROGRESS = 10
    local SOUND_RED_CLAY = 313
    local ACTION_GATHER = 10
    local DURATION_GATHER = 30
    local quest = me:quest(QUEST_RED_CLAY)
    if quest == nil or quest:completed() then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end

    local root = map:model():root()
    if root == nil or not RED_CLAY_PARENT_MAP_NAMES[root:name()] then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local stored_x, stored_y, check
    if param == nil or param == '' then
        stored_x, stored_y, check = 0, 0, 0
    else
        local parts = string_split(param, ',')
        if #parts ~= 3 then
            stored_x, stored_y, check = 0, 0, 0
        else
            stored_x = tonumber(parts[1]) or 0
            stored_y = tonumber(parts[2]) or 0
            check = tonumber(parts[3]) or 0
        end
    end

    if check == 0 then
        quest:param(string.format('%d,%d,1', x, y))
        return
    end

    quest:param(string.format('%d,%d,0', x, y))

    if x == stored_x and y == stored_y then
        return
    end

    local r1 = math.random(1, 1000)
    local s1 = math.random(1, 500)
    local lucky = r1 <= 50 and s1 <= 50
    if lucky then
        if quest:progress() + 1 > RED_CLAY_MAX_PROGRESS then
            me:chat('아~싸~ 적심토다! 근데, 10꾸러미나 들고 있으려니 너무 무겁네. 더이상은 못들 것 같다.')
            return
        end

        me:action(ACTION_GATHER, DURATION_GATHER, 0)
        me:sound(SOUND_RED_CLAY)
        quest:inc_progress(1)
        me:push_achievement(RED_CLAY_ACHIEVEMENT_ID, string.format("적심토 %d꾸러미 보관중", quest:progress()), 6, 1)
        me:chat('아~싸~ 적심토다!')

        local mega_lucky = r1 <= 10 and s1 <= 10
        if mega_lucky then
            sleep(1000)
            me:chat('엇! 그런데, 이 빛은 뭐지? 그리고, 이 따뜻한 기운은...')
            sleep(1000)
            me:chat('윽! 눈부셔!')
            sleep(1000)
            me:chat('희한한 문양이 새겨진 방패로군. 범상치 않아 보이는데?')
            if me:mkitem('여신의방패', 1) ~= nil then
                me:message('여신의 축복이 주변의 적들로부터 당신을 보호합니다.')
                me:hp(me:maxhp())
                me:mp(me:maxmp())
                broadcast(string.format('여신의 가호가 %s의 머리위에 함께 하리라.', me:name()), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
            end
        end
    end
end

--- Chance to find 청심초 when moving on Namgyeong 1-10 (map id 957-966). Quest 192 step 1 or 2.
function cheongsimcho_on_move(me)
    local ITEM_CHEONGSIMCHO = '청심초'
    local CHEONGSIMCHO_MAX = 5
    -- local NAMGYEONG_MAP_ID_MIN = name2map('남경1'):id()
    -- local NAMGYEONG_MAP_ID_MAX = name2map('남경10'):id()
    local NAMGYEONG_MAP_ID_MIN = 0
    local NAMGYEONG_MAP_ID_MAX = 1

    local quest = me:quest(QUEST_ALCOHOLIC_DRINK)
    if quest == nil or quest:completed() then
        return
    end
    local step = quest:step()
    if step < 1 or step > 2 then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    local map_id = map:model():id()
    if map_id < NAMGYEONG_MAP_ID_MIN or map_id > NAMGYEONG_MAP_ID_MAX then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local stored_x, stored_y, check
    if param == nil or param == '' then
        stored_x, stored_y, check = 0, 0, 0
    else
        local parts = string_split(param, ',')
        if #parts ~= 3 then
            stored_x, stored_y, check = 0, 0, 0
        else
            stored_x = tonumber(parts[1]) or 0
            stored_y = tonumber(parts[2]) or 0
            check = tonumber(parts[3]) or 0
        end
    end

    if check == 0 then
        quest:param(string.format('%d,%d,1', x, y))
        return
    end

    quest:param(string.format('%d,%d,0', x, y))

    if x == stored_x and y == stored_y then
        return
    end

    local item = me:item(ITEM_CHEONGSIMCHO)
    local count = (item ~= nil) and item:count() or 0
    if count >= CHEONGSIMCHO_MAX then
        return
    end

    if math.random(1, 100) > 10 then
        return
    end

    if me:mkitem(ITEM_CHEONGSIMCHO, 1) == nil then
        return
    end
    local obj = name2item('청심초')
    if obj ~= nil then
        me:dialog(obj, '청심초를 구했다!', true, true)
    else
        me:chat('청심초를 구했다!')
    end
end

--- Chance to repair Great Wall when moving on 만리장성1-4 (map id 856-859). Consumes 벽돌; 70% success adds progress. Quest 199 step 1.
function greatwall_repair_on_move(me)
    local GREATWALL_ACHIEVEMENT_ID = 21
    local MAP_ID_MIN = 856
    local MAP_ID_MAX = 859

    local quest = me:quest(QUEST_GREATWALL)
    if quest == nil or quest:completed() then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    local map_id = map:model():id()
    if map_id < MAP_ID_MIN or map_id > MAP_ID_MAX then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local stored_x, stored_y, check
    if param == nil or param == '' then
        stored_x, stored_y, check = 0, 0, 0
    else
        local parts = string_split(param, ',')
        if #parts ~= 3 then
            stored_x, stored_y, check = 0, 0, 0
        else
            stored_x = tonumber(parts[1]) or 0
            stored_y = tonumber(parts[2]) or 0
            check = tonumber(parts[3]) or 0
        end
    end

    if check == 0 then
        quest:param(string.format('%d,%d,1', x, y))
        return
    end

    quest:param(string.format('%d,%d,0', x, y))

    if x == stored_x and y == stored_y then
        return
    end

    if math.random(1, 100) > 9 then
        return
    end

    local btn = me:dialog(nil, '엇! 만리장성이 부숴져 가고 있군. 어서 고쳐야 할텐데...', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local brick = me:item('벽돌')
    if brick == nil or brick:count() < 1 then
        me:dialog(nil, '이런.. 벽돌이 없잖아! 벽돌을 사 와야 겠군..', false, true)
        return
    end

    me:rmitem('벽돌', 1, ITEM_DELETE_TYPE.GIVE)
    if math.random(1, 100) <= 70 then
        quest:inc_progress(1)
        me:push_achievement(GREATWALL_ACHIEVEMENT_ID, string.format('만리장성을 %d번 고치다.', quest:progress()), 7, 1)
        me:dialog(nil, '좋아.. 잘 고쳐진 것 같군..', false, true)
    else
        me:dialog(nil, '앗!! 벽돌이 부숴졌잖아! 이런.. 다시해야겠군.', false, true)
    end
end

--- Chance to find 복건성태자 quest toys when moving. Quest 202 step 1. 복건성 -> 상아주사위, 상해1/2/3 -> 오색폭죽, 강서성 -> 청옥팽이. Param stores "x1,y1,c1,x2,y2,c2,x3,y3,c3" for the three regions.
function crown_prince_toys_on_move(me)
    local quest = me:quest(QUEST_FIND_TOYS)
    if quest == nil or quest:step() ~= 1 then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    local map_name = map:model():name()
    local region -- 1 = 복건성 (상아주사위), 2 = 상해1/2/3 (오색폭죽), 3 = 강서성 (청옥팽이)
    local item_name
    local msg
    if map_name == '복건성' then
        region = 1
        item_name = '상아주사위'
        msg = '상아로 만든 예쁜 주사위를 주웠다!'
    elseif map_name == '상해1' or map_name == '상해2' or map_name == '상해3' then
        region = 2
        item_name = '오색폭죽'
        msg = '다섯가지 색 불꽃이 나는 신기한 폭죽을 주웠다!'
    elseif map_name == '강서성' then
        region = 3
        item_name = '청옥팽이'
        msg = '푸른색 옥으로 만든 멋진 팽이를 주웠다!'
    else
        return
    end

    local item = me:item(item_name)
    if item ~= nil and item:count() >= 1 then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local parts = {}
    if param == nil or param == '' then
        for i = 1, 9 do
            parts[i] = '0'
        end
    else
        parts = string_split(param, ',')
        if #parts ~= 9 then
            for i = 1, 9 do
                parts[i] = '0'
            end
        end
    end

    local base = (region - 1) * 3 + 1
    local stored_x = tonumber(parts[base]) or 0
    local stored_y = tonumber(parts[base + 1]) or 0
    local check = tonumber(parts[base + 2]) or 0

    if check == 0 then
        parts[base] = tostring(x)
        parts[base + 1] = tostring(y)
        parts[base + 2] = '1'
        quest:param(table.concat(parts, ','))
        return
    end

    parts[base + 2] = '0'
    quest:param(table.concat(parts, ','))

    if x == stored_x and y == stored_y then
        return
    end

    local r = math.random(1, 100)
    if r < 50 or r > 55 then
        return
    end

    if me:mkitem(item_name, 1) == nil then
        return
    end
    me:chat(msg)
end

--- Chance to find 인삼/동충하초 when moving. Quest 204 (돈유합의달인) step 1 or 2. 국경지대 -> 인삼, 대방성입구 -> 동충하초. Param "x1,y1,c1,x2,y2,c2" for the two maps.
function dongchung_insam_on_move(me)
    local quest = me:quest(QUEST_DONUHAP)
    if quest == nil then
        return
    end
    local step = quest:step()
    if step ~= 1 and step ~= 2 then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    local map_name = map:model():name()
    local region -- 1 = 국경지대 (인삼), 2 = 대방성입구 (동충하초)
    local item_name
    local msg
    if map_name == '국경지대' then
        region = 1
        item_name = '인삼'
        msg = '드디어 인삼을 찾았다! 정말 구하기 힘들구나.'
    elseif map_name == '대방성입구' then
        region = 2
        item_name = '동충하초'
        msg = '드디어 동충하초를 찾았다! 정말 구하기 힘들구나.'
    else
        return
    end

    local item = me:item(item_name)
    if item ~= nil and item:count() >= 1 then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local parts = {}
    if param == nil or param == '' then
        for i = 1, 6 do
            parts[i] = '0'
        end
    else
        parts = string_split(param, ',')
        if #parts ~= 6 then
            for i = 1, 6 do
                parts[i] = '0'
            end
        end
    end

    local base = (region - 1) * 3 + 1
    local stored_x = tonumber(parts[base]) or 0
    local stored_y = tonumber(parts[base + 1]) or 0
    local check = tonumber(parts[base + 2]) or 0

    if check == 0 then
        parts[base] = tostring(x)
        parts[base + 1] = tostring(y)
        parts[base + 2] = '1'
        quest:param(table.concat(parts, ','))
        return
    end

    parts[base + 2] = '0'
    quest:param(table.concat(parts, ','))

    if x == stored_x and y == stored_y then
        return
    end

    local r = math.random(1, 100)
    if r < 50 or r > 53 then
        return
    end

    if me:mkitem(item_name, 1) == nil then
        return
    end
    me:chat(msg)
end

--- Bury 귀신퇴치부적 in 귀기서린집 when moving. Quest 206 step 1 or 3. 10% chance on tile change: consume 1 부적, inc_progress(1). Param "x,y,check" for position.
function ghost_talisman_on_move(me)
    local quest = me:quest(QUEST_GHOST)
    if quest == nil then
        return
    end
    local step = quest:step()
    if step ~= 1 and step ~= 3 then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    if map:model():name() ~= '귀기서린집' then
        return
    end

    local item = me:item('귀신퇴치부적')
    if item == nil or item:count() < 1 then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local parts = {}
    if param == nil or param == '' then
        parts = { '0', '0', '0' }
    else
        parts = string_split(param, ',')
        if #parts ~= 3 then
            parts = { '0', '0', '0' }
        end
    end

    local stored_x = tonumber(parts[1]) or 0
    local stored_y = tonumber(parts[2]) or 0
    local check = tonumber(parts[3]) or 0

    if check == 0 then
        quest:param(string.format('%d,%d,1', x, y))
        return
    end

    quest:param(string.format('%d,%d,0', x, y))

    if x == stored_x and y == stored_y then
        return
    end

    if math.random(1, 100) > 10 then
        return
    end

    me:rmitem('귀신퇴치부적', 1, ITEM_DELETE_TYPE.GIVE)
    quest:inc_progress(1)
    me:dialog(nil, '이 바닥에 부적을 묻으면 귀신의 기운이 약간 약해질 것이다.', false, true)
end

function goddess_dew_on_move(me)
    local MAP_POGYEOMDO = '폭염도'
    local ITEM_DEW = '여신의이슬'
    local DEW_ACHIEVEMENT_ID = 42
    local TOTEM_WATER_MAX = 50

    local quest = me:quest(QUEST_TOTEM_CLOTHES)
    if quest == nil or quest:completed() then
        return
    end
    if quest:step() ~= 1 then
        return
    end

    local map = me:map()
    if map == nil or map:model():name() ~= MAP_POGYEOMDO then
        return
    end

    local item = me:item(ITEM_DEW)
    if item ~= nil and item:count() > 0 then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local stored_x, stored_y, check, water
    if param == nil or param == '' then
        stored_x, stored_y, check, water = 0, 0, 0, 0
    else
        local parts = string_split(param, ',')
        if #parts ~= 4 then
            stored_x, stored_y, check, water = 0, 0, 0, 0
        else
            stored_x = tonumber(parts[1]) or 0
            stored_y = tonumber(parts[2]) or 0
            check = tonumber(parts[3]) or 0
            water = tonumber(parts[4]) or 0
        end
    end
    if water >= TOTEM_WATER_MAX then
        return
    end

    if check == 0 then
        quest:param(string.format('%d,%d,1,%d', x, y, water))
        return
    end

    quest:param(string.format('%d,%d,0,%d', x, y, water))

    if x == stored_x and y == stored_y then
        return
    end

    if math.random(1, 100) > 10 then
        return
    end

    water = water + 1
    quest:param(string.format('%d,%d,0,%d', x, y, water))

    if water == TOTEM_WATER_MAX then
        if me:mkitem(ITEM_DEW, 1) == nil then
            return
        end
        me:push_achievement(DEW_ACHIEVEMENT_ID, '여신의이슬을 만들었다.', 7, 16)
        me:dialog(nil, '투명한 이슬을 얻었다! 여신의 이슬이 완성되었다!', true, true)
        return
    end

    if math.random(1, 10) <= 9 then
        me:push_achievement(DEW_ACHIEVEMENT_ID, string.format('투명한 이슬을 %d개 구했다.', water), 7, 16)
        me:dialog(nil, '투명한 이슬을 발견했다!\n\n조심조심... 투명한 이슬을 담자.', true, true)
    else
        water = water - 1
        quest:param(string.format('%d,%d,0,%d', x, y, water))
        me:dialog(nil, '아뿔사! 투명한 이슬을 흘려버렸다!', true, true)
    end
end

function manrihyang_seed_on_move(me)
    local MAP_ID_MIN = 1100
    local MAP_ID_MAX = 1119
    local ITEM_SEED = '만리향씨앗'
    local SEED_ACHIEVEMENT_ID = 35

    local quest = me:quest(QUEST_DOJAEYOUNG_HERB)
    if quest == nil then
        return
    end
    if quest:step() ~= 6 then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    local map_id = map:model():id()
    if map_id < MAP_ID_MIN or map_id > MAP_ID_MAX then
        return
    end

    local seed_item = me:item(ITEM_SEED)
    if seed_item == nil or seed_item:count() < 1 then
        return
    end

    local x, y = me:position()
    local param = quest:param()
    local stored_x, stored_y, check
    if param == nil or param == '' then
        stored_x, stored_y, check = 0, 0, 0
    else
        local parts = string_split(param, ',')
        if #parts ~= 3 then
            stored_x, stored_y, check = 0, 0, 0
        else
            stored_x = tonumber(parts[1]) or 0
            stored_y = tonumber(parts[2]) or 0
            check = tonumber(parts[3]) or 0
        end
    end

    if check == 0 then
        quest:param(string.format('%d,%d,1', x, y))
        return
    end

    quest:param(string.format('%d,%d,0', x, y))

    if x == stored_x and y == stored_y then
        return
    end

    local r = math.random(1, 100)
    if r < 40 or r > 70 then
        return
    end

    if me:rmitem(ITEM_SEED, 1, ITEM_DELETE_TYPE.GIVE) == nil then
        return
    end

    if math.random(1, 10) <= 9 then
        quest:inc_progress(1)
        local count = quest:progress()
        me:push_achievement(SEED_ACHIEVEMENT_ID, string.format('만리향씨앗 %d번 심다.', count), 7, 3)
        me:dialog(nil, '오! 이곳이 만리향 꽃을 피우기엔 정말 좋은 장소군...\n\n부디 잘 자라서 이쁜꽃을 피워 좋은 향기가 널리 퍼졌으면 좋겠군.')
    else
        me:dialog(nil, '이런! 씨앗이 썩었잖아! 이래선 꽃이 안피겠는걸. 다른 씨앗을 심어야겠다.')
    end
end

--- When at the treasure location (quest param "map,x,y"), give 산신의비단 and set param to "got". QUEST_MOUNTAIN_GOD step 2.
function mountain_treasure_fabric_on_move(me)
    local quest = me:quest(QUEST_MOUNTAIN_GOD)
    if quest == nil or quest:step() ~= 2 then
        return
    end

    local param = quest:param()
    local parts = {}
    for p in string.gmatch(param, '[^,]+') do
        table.insert(parts, p)
    end
    if #parts ~= 3 then
        return
    end

    local map_name = parts[1]
    local target_x = tonumber(parts[2])
    local target_y = tonumber(parts[3])
    if target_x == nil or target_y == nil then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end
    if map:model():name() ~= map_name then
        return
    end

    local x, y = me:position()
    if x ~= target_x or y ~= target_y then
        return
    end

    me:rmitem('산신의보물지도', 1)
    me:mkitem('산신의비단', 1)

    quest:param('got')
    me:dialog(nil, '산신의비단을 발견했다!', false, true)
end

function mountain_treasure_map_on_move(me)
    local quest = me:quest(QUEST_MOUNTAIN_GOD)
    if quest == nil then
        return
    end

    local map = me:map()
    if map == nil then
        return
    end

    local SANSHIN_TREASURE_MAP_IDS = {
        [1287] = true, [1291] = true, [1303] = true, [1313] = true,
        [1327] = true, [1330] = true, [1336] = true, [1344] = true,
        [1347] = true, [1351] = true, [1357] = true, [1360] = true,
        [1366] = true, [1374] = true, [1377] = true,
    }
    local map_id = map:model():id()
    if not SANSHIN_TREASURE_MAP_IDS[map_id] then
        return
    end

    _G._mountain_treasure_map_move = _G._mountain_treasure_map_move or {}
    local key = me:uid()
    local state = _G._mountain_treasure_map_move[key]
    if state == nil then
        state = { x = 0, y = 0, check = 0 }
        _G._mountain_treasure_map_move[key] = state
    end

    local x, y = me:position()

    if state.check == 0 then
        state.x = x
        state.y = y
        state.check = 1
        return
    end

    state.check = 0
    if x == state.x and y == state.y then
        return
    end

    state.x = x
    state.y = y

    local r = math.random(1, 100)
    local rate = math.random(1, 100)
    if r > 3 or rate > 5 then
        return
    end

    if me:item('산신의보물지도') ~= nil then
        return
    end

    if me:mkitem('산신의보물지도', 1) == nil then
        return
    end

    me:dialog(name2item('산신의보물지도'), '산신의보물지도를 발견했다!', false, false)
end

function on_move(me)
    any_action(me)

    if me:is(OBJECT_TYPE.CHARACTER) then
        red_clay_on_move(me)
        cheongsimcho_on_move(me)
        greatwall_repair_on_move(me)
        crown_prince_toys_on_move(me)
        dongchung_insam_on_move(me)
        ghost_talisman_on_move(me)
        goddess_dew_on_move(me)
        manrihyang_seed_on_move(me)
        mountain_treasure_map_on_move(me)
        mountain_treasure_fabric_on_move(me)
    end
end

function on_direction(me)
    any_action(me)
end

function any_action(me)
    if me:isbuff('운기') then
        me:unbuff('운기')
    end
end

function on_chat(me, message, shout)
    if string.sub(message, 1, 1) == '/' then
        message = string.sub(message, 2, string.len(message))
        args = string_split(message, ' ')

        local cmd = args[1]
        if command_funcs[cmd] == nil then
            return false
        end
        
        local cmd_data = command_funcs[cmd]
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
end

function on_npc_chat(me, message, shout)
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

    -- Gatekeeper NPC name -> { totem name (e.g. "청룡"), totem_key (e.g. "dragon") } for "~참가" chat.
    local GATEKEEPER_BY_NAME = {
        ["주작성문지기"] = { "주작", "bird" },
        ["청룡성문지기"] = { "청룡", "dragon" },
        ["현무성문지기"] = { "현무", "turtle" },
        ["백호성문지기"] = { "백호", "tiger" },
    }

    local function run_gatekeeper_entrance(me, npc, totem_name_kr, totem_key)
        local clan = me:clan()
        if not clan then
            me:dialog(npc, '가입된 문파가 없습니다.')
            return true
        end
        if me:state() == STATE.GHOST then
            me:dialog(npc, '유령은 참가할 수 없습니다.')
            return true
        end
        local occupant = (_G.clan_castle_occupant or {})[totem_key] or ''
        local siege_start = _G.clan_siege_start or 0
        local siege_map = _G.clan_siege_map or ''
        local clan_name = clan:name()
        local castle_name = totem_name_kr .. '성'
        local map_entrance = name2map(totem_name_kr .. '성입구')
        local map_inner = name2map(totem_name_kr .. '의성')
        if not map_entrance then
            me:dialog(npc, '입장할 수 있는 맵이 없습니다.')
            return true
        end
        if clan_name == occupant then
            if siege_start == 0 and map_inner then
                me:map(map_inner, math.random(11, 17), math.random(4, 11))
            else
                me:map(map_entrance, math.random(49, 57), math.random(145, 148))
            end
            return true
        end
        if siege_map == castle_name then
            me:map(map_entrance, math.random(49, 57), math.random(145, 148))
            return true
        end
        me:dialog(npc, string.format('현재 %s 공성이 진행중이지 않습니다.', castle_name))
        return true
    end
    
    local regex_handlers = {
        { pattern = CHAT_REGEX.BUY, condition = function(npc)
            local model = npc:model()
            local sell = model:sell()
            return #sell > 0
        end, func = function(npc, params)
            local name = params.name
            local count = 1
            if params.count ~= nil then
                count = tonumber(params.count)
            end
            return npc_sell_item(me, npc, name, count)
        end },
        { pattern = CHAT_REGEX.SELL, condition = function(npc)
            local model = npc:model()
            local buy = model:buy()
            return buy ~= nil
        end, func = function(npc, params)
            local name = params.name
            local count = nil
            if params.all ~= nil then
                count = nil
            elseif params.count ~= nil then
                count = tonumber(params.count)
            else
                count = 1
            end
            return npc_buy_item(me, npc, name, count)
        end },
        { pattern = CHAT_REGEX.REPAIR, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.REPAIR) == NPC_INTERACTION.REPAIR
        end, func = function(npc, params)
            if params.all ~= nil then
                return npc_repair(me, npc, nil)
            elseif params.name ~= nil then
                return npc_repair(me, npc, params.name)
            else
                return false
            end
        end },
        { pattern = CHAT_REGEX.DEPOSIT_MONEY, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end, func = function(npc, params)
            local money = nil
            if params.all ~= nil then
                money = me:money()
            elseif params.money ~= nil then
                money = tonumber(params.money)
            else
                return false
            end

            return npc_deposit_money(me, npc, money)
        end },
        { pattern = CHAT_REGEX.WITHDRAW_MONEY, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end, func = function(npc, params)
            local money = nil
            if params.all ~= nil then
                money = nil
            elseif params.money ~= nil then
                money = tonumber(params.money)
            else
                return false
            end

            return npc_withdraw_money(me, npc, money)
        end },
        { pattern = CHAT_REGEX.STORE_ITEM, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            local name = params.name
            local count = nil
            if params.all ~= nil then
                count = nil
            elseif params.count ~= nil then
                count = tonumber(params.count)
            else
                count = 1
            end
            return npc_store_item(me, npc, name, count)
        end },
        { pattern = CHAT_REGEX.RETRIEVE_ITEM, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            local name = params.name
            local count = nil
            if params.all ~= nil then
                count = nil
            elseif params.count ~= nil then
                count = tonumber(params.count)
            else
                count = 1
            end
            return npc_retrieve_item(me, npc, name, count)
        end },
        { pattern = CHAT_REGEX.SELL_LIST, condition = function(npc)
            local model = npc:model()
            local sell = model:sell()
            return #sell > 0
        end, func = function(npc, params)
            return npc_sell_item_list(me, npc)
        end },
        { pattern = CHAT_REGEX.BUY_LIST, condition = function(npc)
            local model = npc:model()
            local buy = model:buy()
            return buy ~= nil
        end, func = function(npc, params)
            return npc_buy_item_list(me, npc)
        end },
        { pattern = CHAT_REGEX.SELL_PRICE, condition = function(npc)
            local model = npc:model()
            local sell = model:sell()
            return #sell > 0
        end, func = function(npc, params)
            local name = params.name
            return npc_sell_item_price(me, npc, name)
        end },
        { pattern = CHAT_REGEX.BUY_PRICE, condition = function(npc)
            local model = npc:model()
            local buy = model:buy()
            return buy ~= nil
        end, func = function(npc, params)
            local name = params.name
            return npc_buy_item_price(me, npc, name)
        end },
        { pattern = CHAT_REGEX.DEPOSITED_MONEY, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end, func = function(npc, params)
            return npc_deposited_money(me, npc)
        end },
        { pattern = CHAT_REGEX.RENAME_WEAPON, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.RENAME) == NPC_INTERACTION.RENAME
        end, func = function(npc, params)
            local from = params.weapon
            local to = params.name
            return npc_rename_weapon(me, npc, from, to)
        end },
        { pattern = CHAT_REGEX.HOLD_ITEM_LIST, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            return npc_store_item_list(me, npc)
        end },
        { pattern = CHAT_REGEX.HOLD_ITEM_COUNT, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            local name = params.name
            return npc_store_item_count(me, npc, name)
        end },
        { pattern = CHAT_REGEX.REVIVE, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.REVIVE) == NPC_INTERACTION.REVIVE
        end, func = function(npc, params)
            local discourteous = params.no ~= nil
            return npc_revive(me, npc, discourteous)
        end },
        { pattern = CHAT_REGEX.APPRECIATE, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.REVIVE) == NPC_INTERACTION.REVIVE
        end, func = function(npc, params)
            return npc_appreciate(me, npc)
        end },
        { pattern = CHAT_REGEX.JOIN_CASTLE, condition = function(npc)
            local name = npc:model():name()
            return GATEKEEPER_BY_NAME[name] ~= nil
        end, func = function(npc, params)
            local name = npc:model():name()
            local info = GATEKEEPER_BY_NAME[name]
            return run_gatekeeper_entrance(me, npc, info[1], info[2])
        end },
        { pattern = CHAT_REGEX.BLACK_FLAG, condition = function(npc)
            return npc:model():name() == '장안성대장간'
        end, func = function(npc, params)
            if me:dialog(npc, '아니, 내가 검정깃발을 가지고 있다는걸 어떻게 알았나.. 으음...', false, true) == DIALOG_RESULT.QUIT then
                return true
            end
            if me:dialog(npc, '그냥 줄순 없고.. 5000전만 내게. 그럼 검정깃발을 하나 주지.', true, true) == DIALOG_RESULT.QUIT then
                return true
            end
            local selected, button = me:list(npc, '어때? 5000전에 검정깃발 하나 사길 텐가?', { '네, 주십시오.', '안 살래요' })
            if button == DIALOG_RESULT.QUIT then
                return true
            end
            if selected ~= 0 then
                return true
            end
            local BLACK_FLAG_PRICE = 5000
            if me:money() < BLACK_FLAG_PRICE then
                me:dialog(npc, '돈이 모자랍니다.', false, true)
                return true
            end
            if me:mkitem('검정깃발', 1) == nil then
                me:dialog(npc, '공간이 부족합니다.', false, true)
                return true
            end
            me:money(me:money() - BLACK_FLAG_PRICE)
            me:dialog(npc, '검정깃발을 받았습니다.', false, false)
            return true
        end },
    }

    for _, handler in ipairs(regex_handlers) do
        local params = regex(handler.pattern, message)
        if params ~= nil then
            for _, npc in ipairs(npcs) do
                if handler.condition ~= nil then
                    if not handler.condition(npc) then
                        goto continue
                    end
                end
                if handler.func(npc, params) then
                    return true
                end
                ::continue::
            end
        end
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

function on_login(me, first_login)
    if first_login then
        me:push_achievement(0, make_baram_birth_label() .. " 생", 0, 47)
    end
    if me:birthday() ~= nil then
        return
    end

    local npc = name2npc('낙랑')
    local button = nil
::BIRTHDAY_DIALOG_1::
    button = me:dialog(npc, '대단히 중요하니 끝까지 읽어주세요! 빈번히 발생하는 아이디 해킹을 미연에 방지하기 위해 또 하나의 2차 비밀번호를 정해야 합니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

::BIRTHDAY_DIALOG_2::
    button = me:dialog(npc, '이 2차 비밀번호는 한번 결정을 하시면 변경이 불가능 합니다. 이 2차 비밀번호는 자신의 게임 접속 비밀번호를 바꿀 때에만 사용합니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_1
    end

::BIRTHDAY_DIALOG_3::
    button = me:dialog(npc, '이 2차 비밀번호는 자신의 생년월일로 해주세요. 예를 들면 자신의 생일이 92년 04월 17일이면, 920417으로 입력해주세요. (반드시 6자리)', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_2
    end

::BIRTHDAY_DIALOG_4::
    button = me:dialog(npc, '이 2차 비밀번호는 분실하실 이유가 없기 때문에(자신의 생년월일이므로) 이 2차 비밀번호를 모르시는 분은 해당 아이디에 대한 소유권을 박탈합니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_3
    end

::BIRTHDAY_DIALOG_5::
    button = me:dialog(npc, '이 2차 비밀번호를 모르시고 처음의 비밀번호 분실신고를 하셔도 저희는 가르쳐 드릴수가 없습니다. 그 만큼 중요한 것이오니 신중히 그리고 정확히 입력해주세요.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_4
    end

::BIRTHDAY_DIALOG_6::
    button = me:dialog(npc, '또한 이 두번째 비밀번호(생년월일)를 입력하지 않으시고 다음에 비밀번호 분실신고를 하셔도 저희는 해드릴수가 없으니 반드시 두번째 비번을 입력하시기 바랍니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_5
    end

::BIRTHDAY_INPUT::
    local birthday = me:input(npc, '자신의 생년월일을 입력해 주세요.', '저의 생년월일은', '입니다.', 6, true)
    if birthday == DIALOG_RESULT.QUIT then
        goto BIRTHDAY_INPUT
    end
    if birthday == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_6
    end

    ::BIRTHDAY_MUST_SET::
    if birthday == '' or #birthday ~= 6 then
        button = me:dialog(npc, '두번째 비밀번호를 정하지 않으면 게임을 할 수 없습니다. 반드시 해주세요.', true, true)
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

    local sel, list_btn = me:list(npc, string.format('당신의 생년월일이 %s가 맞습니까?', birthday), {'예', '아니오'}, true)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if list_btn == DIALOG_RESULT.PREV or sel == nil then
        goto BIRTHDAY_INPUT
    end

    if sel == 0 then
        me:birthday(birthday)
        return
    end

    goto BIRTHDAY_MUST_SET
end