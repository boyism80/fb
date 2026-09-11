-- mob: 술취한호랑이

local festival = require('lib.festival')

local WINE = '국화주'
local RESET_X = 52
local RESET_Y = 247
local CHAT = '국화주 한 병 주면 안 잡아먹지!!'

local busy = {}
local primed = {}

local function chat(me)
    local oid = tostring(me:oid())
    if primed[oid] then
        return
    end
    primed[oid] = true
    me:chat(CHAT)
end

local function handle_hit(me, you)
    chat(me)

    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not festival.is('중양절') then
        return
    end

    local oid = tostring(me:oid())
    if busy[oid] then
        return
    end
    busy[oid] = true

    if you:has_items(WINE, 1) then
        you:rmitem(WINE, 1, ITEM_DELETE_TYPE.REMOVED)
        you:dialog(me, '흐흐흐.. 맛있는 국화주로군...', { prev = false, next = true })
    else
        local map = me:map()
        if map ~= nil then
            you:map(map, RESET_X, RESET_Y)
        end
    end

    busy[oid] = nil
end

return {
    on_mob_action = function(me)
        chat(me)
    end,

    on_mob_damaged = function(me, you)
        handle_hit(me, you)
    end,

    on_mob_spell_hit = function(me, you, spell)
        handle_hit(me, you)
        return false
    end,
}
