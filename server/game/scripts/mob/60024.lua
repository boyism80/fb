-- mob: 술취한호랑이
-- On hit during 중양절: steal 국화주, or warp player to 약수터 entrance.

local festival = require('lib.festival')

local WINE = '국화주'
local RESET_X = 52
local RESET_Y = 247
local CHAT = '국화주 한 병 주면 안 잡아먹지!!'

-- Per-mob busy flag while dialog runs (keyed by oid string).
local busy = {}

local function handle_hit(me, you)
    me:invincible(true)
    me:chat(CHAT)

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
    on_mob_damaged = function(me, you)
        handle_hit(me, you)
    end,

    on_mob_spell_hit = function(me, you, spell)
        -- Spell pipelines may cancel before damage_to; steal/warp here too.
        handle_hit(me, you)
        me:hp(me:maxhp())
        me:invincible(true)
        return false
    end,
}
