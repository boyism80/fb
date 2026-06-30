local M = {}

M.QUEST_NAKRANG_HUNT = 1
M.QUEST_KING_JANGAN = 100
M.QUEST_KING_BUYEO = 101
M.QUEST_KING_GOGURYEO = 102
M.QUEST_CROCODILE_BLOOD = 155
M.QUEST_NAMGUN = 180
M.QUEST_BONG_BOOK = 181
M.QUEST_RED_CLAY = 182
M.QUEST_TANGTANG = 183
M.QUEST_JUNJUN = 184
M.QUEST_JINJIN = 185
M.QUEST_TUNGTUNG = 186
M.QUEST_TONGTONG = 187
M.QUEST_SAILOR = 188
M.QUEST_MEOKYEOM = 189
M.QUEST_LIGHTHOUSE = 190
M.QUEST_TOTEM_CLOTHES = 191
M.QUEST_ALCOHOLIC_DRINK = 192
M.QUEST_MARTIAL = 193
M.QUEST_SICK_CHILD = 194
M.QUEST_HWANGBIYEON = 195
M.QUEST_DOJAEYOUNG_HERB = 196
M.QUEST_SAMJEONSIN = 197
M.QUEST_GREATWALL = 199
M.QUEST_HOO_KILL = 200
M.QUEST_SELL_DOLL = 201
M.QUEST_FIND_TOYS = 202
M.QUEST_JOWANG = 203
M.QUEST_DONUHAP = 204
M.QUEST_PRINCESS_RING = 205
M.QUEST_GHOST = 206
M.QUEST_OXYGEN = 207
M.QUEST_WATER_RING = 208
M.QUEST_CLEAR_SHIELD = 209
M.QUEST_DRAGON_KING = 210
M.QUEST_WATER_BOTTLE = 211
M.QUEST_PYOSIN = 212
M.QUEST_HOLYTREE = 213
M.QUEST_RABBIT_LIVER = 214
M.QUEST_STRONGBOX = 216
M.QUEST_GOOSE_EGG = 217
M.QUEST_MOUNTAIN_GOD = 218
M.QUEST_DOTAEYEON = 219
M.QUEST_WOOGAPUNG = 220
M.QUEST_HATAEHYUN = 221
M.QUEST_NAKRANG_INTRO = 224
M.QUEST_NAKRANG2 = 225
M.QUEST_NAKRANG3 = 226
M.QUEST_NAKRANG4 = 227
M.QUEST_NAKRANG5 = 228
M.QUEST_NAKRANG7 = 229
M.QUEST_PROMOTION_3RD = 230
M.QUEST_MUTA = 231
M.QUEST_SHARK_WEAPON = 232
M.QUEST_BAEK_MONGYEON = 233
M.QUEST_ANCIENT_RELIC = 234
M.QUEST_JINHWANG = 235
M.QUEST_DOJAECHUN = 236
M.QUEST_PAMASPIRI = 237
M.QUEST_BEGINNER_PATH = 238
M.QUEST_BAEKRIHYANG = 239
M.QUEST_JANGAJI = 240
M.QUEST_CHOOMO = 241
M.QUEST_MADONG_APPLY = 242
M.QUEST_CIDEQUEST = 243
M.QUEST_GMD_SAVED = 244
M.QUEST_BLUEEAR = 245
M.QUEST_SILENCE = 246
M.QUEST_JURY = 247
M.QUEST_WOLF_CURSE = 248
M.QUEST_MAGIC_BALL = 249
M.QUEST_TOLUI_DAY = 250
M.QUEST_DETECTIVE = 251
M.QUEST_WANDO = 252
M.QUEST_JAPAN_LEGEND_WEAPON = 253
M.QUEST_SKULL_NECKLACE = 254
M.QUEST_SKULL_NECKLACE_1 = 255
M.QUEST_SKULL_NECKLACE_2 = 256
M.QUEST_SKULL_NECKLACE_3 = 257
M.QUEST_SKULL_NECKLACE_4 = 258
M.QUEST_SKULL_NECKLACE_5 = 259
M.QUEST_SKULL_NECKLACE_6 = 260
M.QUEST_SKULL_NECKLACE_7 = 261
M.QUEST_SKULL_NECKLACE_8 = 262
M.QUEST_SKULL_NECKLACE_9 = 263
M.QUEST_HWAHWA = 264
M.QUEST_JINGOGYUN = 265
M.QUEST_HWAHWA_SMILE = 266

local interaction
local function string_split(self, delimiter)
    if interaction == nil then
        interaction = require('lib.interaction')
    end
    return interaction.string_split(self, delimiter)
end

function M.red_clay_on_move(me)
    local RED_CLAY_ACHIEVEMENT_ID = 22
    local RED_CLAY_PARENT_MAP_NAMES = { ['고균도'] = true, ['가릉도'] = true, ['폭염도'] = true }
    local RED_CLAY_MAX_PROGRESS = 10
    local SOUND_RED_CLAY = 313
    local ACTION_GATHER = 10
    local DURATION_GATHER = 30
    local quest = me:quest(M.QUEST_RED_CLAY)
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
function M.cheongsimcho_on_move(me)
    local ITEM_CHEONGSIMCHO = '청심초'
    local CHEONGSIMCHO_MAX = 5
    local NAMGYEONG_MAP_ID_MIN = 957
    local NAMGYEONG_MAP_ID_MAX = 966

    local quest = me:quest(M.QUEST_ALCOHOLIC_DRINK)
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
function M.greatwall_repair_on_move(me)
    local GREATWALL_ACHIEVEMENT_ID = 21
    local MAP_ID_MIN = 856
    local MAP_ID_MAX = 859

    local quest = me:quest(M.QUEST_GREATWALL)
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
function M.crown_prince_toys_on_move(me)
    local quest = me:quest(M.QUEST_FIND_TOYS)
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
function M.dongchung_insam_on_move(me)
    local quest = me:quest(M.QUEST_DONUHAP)
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
function M.ghost_talisman_on_move(me)
    local quest = me:quest(M.QUEST_GHOST)
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

function M.goddess_dew_on_move(me)
    local MAP_POGYEOMDO = '폭염도'
    local ITEM_DEW = '여신의이슬'
    local DEW_ACHIEVEMENT_ID = 42
    local TOTEM_WATER_MAX = 50

    local quest = me:quest(M.QUEST_TOTEM_CLOTHES)
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

function M.manrihyang_seed_on_move(me)
    local MAP_ID_MIN = 1100
    local MAP_ID_MAX = 1119
    local ITEM_SEED = '만리향씨앗'
    local SEED_ACHIEVEMENT_ID = 35

    local quest = me:quest(M.QUEST_DOJAEYOUNG_HERB)
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

function M.mountain_treasure_map_on_move(me)
    local quest = me:quest(M.QUEST_MOUNTAIN_GOD)
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

--- When at the treasure location (quest param "map,x,y"), give 산신의비단 and set param to "got". M.QUEST_MOUNTAIN_GOD step 2.
function M.mountain_treasure_fabric_on_move(me)
    local quest = me:quest(M.QUEST_MOUNTAIN_GOD)
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

function M.king_on_mob_die(me, you)
    local killed_name = me:model():name()
    for _, qid in ipairs({ 100, 101, 102 }) do
        local quest = you:quest(qid)
        if quest ~= nil and quest:step() == 1 then
            local param = quest:param() or ''
            local mob_name = param:match('^([^,]+)') or param
            if mob_name == killed_name and quest:progress() < 1 then
                quest:inc_progress(1)
                break
            end
        end
    end
end

function M.king_dialog(me, npc, opts)
    if me:level() < opts.min_level then
        me:dialog(npc, '감히 여기가 어디라고!', false, true)
        return
    end
    if opts.nation ~= nil and me:nation() ~= opts.nation then
        me:dialog(npc, opts.nation_reject_msg or '여기는 왕이 계시는 곳입니다.', false, true)
        return
    end

    local quest = me:quest(opts.quest_id)
    local btn

    if quest == nil then
        ::KING_START0::
        btn = me:dialog(npc, '무례하게 폐하께 직접! 폐하께 전할 말씀은 나를 통해서 하시오!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::KING_START1::
        btn = me:dialog(npc, '폐하게 임무를 받으려고 하시는 겁니까? 임무를 완수하면 많은 경험치를 받으시겠지만, 하지 못하면 형벌을 받게 된다오!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto KING_START0
        end
        local sel, lb = me:list(npc, '그래도 임무를 받으시려오?', { '네, 받겠습니다.', '무서워서 안되겠어요.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end

        local level = me:level()
        local pool = opts.pool_fn(level)
        if pool == nil or #pool == 0 then
            me:dialog(npc, '임무를 부여할 수 없소.', false, true)
            return
        end
        local idx = math.random(1, #pool)
        local mob_name = pool[idx]
        local mob_model = name2mob(mob_name)
        if mob_model == nil then
            me:dialog(npc, '임무를 부여할 수 없소.', false, true)
            return
        end

        quest = me:start_quest(opts.quest_id)
        if quest == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest:step(1)
        quest:param(mob_name)
        quest:progress(0)
        me:push_achievement(opts.achievement_id, opts.king_name .. '에게 ' .. name_with(mob_name, '을', '를') .. ' 잡는 임무를 받음', 6, 17)
        me:dialog(npc, '어명이오! ' .. name_with(mob_name, '을', '를') .. ' 잡으라는 폐하의 말씀이 있으셨소! 임무를 수행한 후에는 시간을 지체하지 말고 바로 황궁으로 돌아오시오!', false, true)
        return
    end

    if quest:step() == 0 then
        ::KING_START0B::
        btn = me:dialog(npc, '무례하게 폐하께 직접! 폐하께 전할 말씀은 나를 통해서 하시오!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::KING_START1B::
        btn = me:dialog(npc, '폐하게 임무를 받으려고 하시는 겁니까? 임무를 완수하면 많은 경험치를 받으시겠지만, 하지 못하면 형벌을 받게 된다오!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto KING_START0B
        end
        local sel, lb = me:list(npc, '그래도 임무를 받으시려오?', { '네, 받겠습니다.', '무서워서 안되겠어요.' })
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel ~= 0 then
            return
        end

        local level = me:level()
        local pool = opts.pool_fn(level)
        if pool == nil or #pool == 0 then
            me:dialog(npc, '임무를 부여할 수 없소.', false, true)
            return
        end
        local idx = math.random(1, #pool)
        local mob_name = pool[idx]
        local mob_model = name2mob(mob_name)
        if mob_model == nil then
            me:dialog(npc, '임무를 부여할 수 없소.', false, true)
            return
        end

        quest:step(1)
        quest:param(mob_name)
        quest:progress(0)
        me:push_achievement(opts.achievement_id, opts.king_name .. '에게 ' .. name_with(mob_name, '을', '를') .. ' 잡는 임무를 받음', 6, 17)
        me:dialog(npc, '어명이오! ' .. name_with(mob_name, '을', '를') .. ' 잡으라는 폐하의 말씀이 있으셨소! 임무를 수행한 후에는 시간을 지체하지 말고 바로 황궁으로 돌아오시오!', false, true)
        return
    end

    if quest:step() == 1 then
        if quest:progress() == 0 then
            local mob_name = name_with(quest:param() or '', '을', '를')
            btn = me:dialog(npc, string.format('네 이놈! %s 잡을 어명을 받고서 %s 잡지 않았구나!', mob_name, mob_name), false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end

            local sel, btn = me:list(npc, '아직 임무를 완수하지도 않았으면서 왜 찾아왔느냐?', {'임무를 취소시켜 주십시오.', '임무를 완수하고 다시 오겠습니다.'})
            if btn == DIALOG_RESULT.QUIT then
                return
            end

            if sel == 1 then
                return
            end
            
            me:dialog(npc, '감히 폐하의 임무를 취소해달라고? 이놈에게 형벌을 가하라!', true, true)
            me:sound(69)
            me:effect(13)
            me:buff('왕의저주', opts.curse_sec)
            me:erase_achievement(opts.achievement_id)
            quest:step(0)
            quest:param('')
            quest:progress(0)
            me:dialog(npc, '이 형벌로 너의 임무가 지워졌으니, 다시 임무를 받을 수 있을 것이다.', false, true)
            return
        end

        local mob_name = quest:param() or ''
        local mob_model = name2mob(mob_name)
        local base_exp = (mob_model ~= nil) and mob_model:exp() or 0
        local rate = exp_multiplier()
        local exp_amount = math.floor(base_exp * 10 * rate)
        btn = me:dialog(npc, '어명을 받든 공을 높이 사신 폐하께서 그대에게 경험치 ' .. tostring(exp_amount) .. ' 을 하사하십니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:exp(me:exp() + exp_amount)
        quest:step(0)
        quest:param('')
        quest:progress(0)
        me:dialog(npc, '어명을 받든 공을 높이 사신 폐하께서 그대에게 경험치를 하사하셨소.', false, true)
    end
end

return M
