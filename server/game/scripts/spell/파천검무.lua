-- 파천검무 캐스팅
local spell = require('lib.spell')

function ON_CAST_2022(me, sp)
    local function failed(me)
        me:message(string.format('%s 실패', sp:name()))
    end

    local sound = 501
    local effect = 167
    local hp = me:hp()//2
    local mp = 180
    local damage = me:hp()
    local message = '파천검무'

    local map = me:map()
    if map == nil then
        return failed(me)
    end

    if not map:movable(me, 5) then
        return failed(me)
    end

    local nears = spell.nears_exclude_item(me)
    local x, y = me:position()
    local direction = me:direction()
    local targets = {}
    for i = 1, 4 do
        local obj = spell.front_obj(x, y, direction, i, nears, OBJECT_TYPE.LIFE)
        if obj ~= nil then
            table.insert(targets, obj)
        end
    end

    if #targets > 0 then
        me:position(me:front_position(5))
        spell.attack_cast(me, targets, sp, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect})
        return
    else
        return failed(me)
    end
end