-- mob: 술취한호랑이

local festival = require('lib.festival')

local WINE = '국화주'
local RESET_X = 52
local RESET_Y = 247
local CHAT = '국화주 한 병 주면 안 잡아먹지!!'
local CHAT_CHANCE = 30

local function handle_hit(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if me:hp() == 0 then
        return
    end
    if not festival.is('중양절') then
        return
    end

    if you:has_items(WINE, 1) then
        you:rmitem(WINE, 1, ITEM_DELETE_TYPE.REMOVED)
        you:dialog(me, '흐흐흐.. 맛있는 국화주로군...', { prev = false, next = false, immediate = true })
    else
        local map = me:map()
        if map ~= nil then
            you:map(map, RESET_X, RESET_Y)
        end
    end
end

return {
    on_mob_action = function(me)
        if math.random(1, CHAT_CHANCE) == 1 then
            me:chat(CHAT)
        end
    end,

    on_mob_damaged = function(me, you)
        handle_hit(me, you)
    end,

    on_mob_spell_hit = function(me, you, spell)
        handle_hit(me, you)
        return false
    end,
}
