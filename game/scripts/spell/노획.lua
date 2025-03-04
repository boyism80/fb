-- 노획 캐스팅
function ON_CAST_2008(me, spell)
    local map = me:map()
    if map == nil then
        return
    end

    local mp = 30
    local effect = nil
    local sound = 37
    if not spell_cast(me, me, spell, mp, sound, effect) then
        return
    end

    local x, y = me:front_position()
    local items = map:belows(x, y, OBJECT_TYPE_ITEM)
    me:gain(table.unpack(items))
    me:chat('노획', CHAT_TYPE_BLUE, false)
end