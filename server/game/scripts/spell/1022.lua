-- spell: 극'백호참
-- 극'백호참 캐스팅
local spell = require('lib.spell')

function ON_CAST_1022(me, sp)
    local effect = 164
    local sound = 88
    local hp = me:hp()//2
    local mp = 50
    local damage = me:hp()
    local message = '극\'백호참'

    local targets = {}
    local x, y = me:position()
    local direction = me:direction()

    local positions = {}
    if direction == DIRECTION.LEFT then
        table.insert(positions, {x-1, y  })
        table.insert(positions, {x-2, y-1})
        table.insert(positions, {x-2, y  })
        table.insert(positions, {x-2, y+1})
        table.insert(positions, {x-3, y-2})
        table.insert(positions, {x-3, y-1})
        table.insert(positions, {x-3, y  })
        table.insert(positions, {x-3, y+1})
        table.insert(positions, {x-3, y+2})
    elseif direction == DIRECTION.TOP then
        table.insert(positions, {x  , y-1})
        table.insert(positions, {x-1, y-2})
        table.insert(positions, {x  , y-2})
        table.insert(positions, {x+1, y-2})
        table.insert(positions, {x-2, y-3})
        table.insert(positions, {x-1, y-3})
        table.insert(positions, {x  , y-3})
        table.insert(positions, {x+1, y-3})
        table.insert(positions, {x+2, y-3})
    elseif direction == DIRECTION.RIGHT then
        table.insert(positions, {x+1, y  })
        table.insert(positions, {x+2, y-1})
        table.insert(positions, {x+2, y  })
        table.insert(positions, {x+2, y+1})
        table.insert(positions, {x+3, y-2})
        table.insert(positions, {x+3, y-1})
        table.insert(positions, {x+3, y  })
        table.insert(positions, {x+3, y+1})
        table.insert(positions, {x+3, y+2})
    else
        table.insert(positions, {x  , y+1})
        table.insert(positions, {x-1, y+2})
        table.insert(positions, {x  , y+2})
        table.insert(positions, {x+1, y+2})
        table.insert(positions, {x-2, y+3})
        table.insert(positions, {x-1, y+3})
        table.insert(positions, {x  , y+3})
        table.insert(positions, {x+1, y+3})
        table.insert(positions, {x+2, y+3})
    end
    for _, obj in pairs(me:nears(OBJECT_TYPE.LIFE)) do
        local obj_x, obj_y = obj:position()
        for _, position in pairs(positions) do
            if obj_x == position[1] and obj_y == position[2] then
                table.insert(targets, obj)
                break
            end
        end
    end

    if not spell.attack_cast(me, targets, sp, {hp=hp, mp=mp, damage=damage, message=message, sound=sound, effect=effect}) then
        return
    end
end
