-- map: 약수터
-- Warp tiles (54-60,13) -> on_jungyang_spring
local festival = require('lib.festival')

local MOB_DRUNK_TIGER = 60024
local TIGER_CHAT = '국화주 한 병 주면 안 잡아먹지!!'

local function ensure_tiger(map)
    if map == nil then
        return
    end
    for _, mob in pairs(map:objects(OBJECT_TYPE.MOB)) do
        if mob:model():id() == MOB_DRUNK_TIGER then
            mob:invincible(true)
            mob:chat(TIGER_CHAT)
            return
        end
    end
    if not festival.is('중양절') then
        return
    end
    local mob = map:spawn_mob(MOB_DRUNK_TIGER, 55, 105, DIRECTION.BOTTOM)
    if mob ~= nil then
        mob:invincible(true)
        mob:chat(TIGER_CHAT)
    end
end

return {
    on_map_enter = function(me)
        local map = me:map()
        ensure_tiger(map)
    end,

    on_jungyang_spring = function(me)
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
    end,
}
