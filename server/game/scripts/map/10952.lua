-- map: 약수터
-- Spring tiles (54-60, 13): dest=script warps -> on_jungyang_spring

local festival = require('lib.festival')

local MOB_DRUNK_TIGER = 60024
local TIGER_CHAT = '국화주 한 병 주면 안 잡아먹지!!'
local TARGET_COUNT = 50
local SPACING = 14

local function count_tigers(map)
    local n = 0
    for _, mob in pairs(map:objects(OBJECT_TYPE.MOB)) do
        if mob:model():id() == MOB_DRUNK_TIGER then
            n = n + 1
            mob:invincible(true)
        end
    end
    return n
end

local function spawn_one(map, x, y)
    local mob = map:spawn_mob(MOB_DRUNK_TIGER, x, y, DIRECTION.BOTTOM)
    if mob == nil then
        mob = map:spawn_mob('술취한호랑이', x, y, DIRECTION.BOTTOM)
    end
    if mob == nil then
        return false
    end
    mob:invincible(true)
    mob:chat(TIGER_CHAT)
    return true
end

local function ensure_tigers(map)
    if map == nil then
        return
    end
    if not festival.is('중양절') then
        return
    end

    local need = TARGET_COUNT - count_tigers(map)
    if need <= 0 then
        return
    end

    local w = map:width()
    local h = map:height()
    if w == nil or h == nil or w < 1 or h < 1 then
        return
    end

    local margin = 10
    for y = margin, h - margin - 1, SPACING do
        for x = margin, w - margin - 1, SPACING do
            if need <= 0 then
                return
            end
            if not map:block(x, y) and map:at(x, y, OBJECT_TYPE.MOB) == nil then
                if spawn_one(map, x, y) then
                    need = need - 1
                end
            end
        end
    end

    for _ = 1, need * 30 do
        if need <= 0 then
            break
        end
        local x = math.random(0, w - 1)
        local y = math.random(0, h - 1)
        if not map:block(x, y) and map:at(x, y, OBJECT_TYPE.MOB) == nil then
            if spawn_one(map, x, y) then
                need = need - 1
            end
        end
    end
end

local function on_jungyang_spring(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not festival.is('중양절') then
        return
    end
    if me:has_items('약신의물병', 1) then
        return
    end

    local sel, list_btn = me:list(me, '약수를 드시겠습니까?', {
        '네',
        '아니오..',
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel ~= 1 then
        return
    end

    if me:mkitem('약신의물병', 1) == nil then
        me:dialog(me, '소지품이 가득 차서 약신의물병을 받을 수 없습니다.', { prev = false, next = false })
        return
    end

    local btn = me:dialog(me, '매달 정해진 한가지씩의 아이템을 모아서 12개를 모두 모으시면 좋은일이 생길꺼에요. 중양절엔 약신의물병입니다.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
end

return {
    on_map_enter = function(me)
        ensure_tigers(me:map())
    end,

    on_jungyang_spring = on_jungyang_spring,
}
