-- 극'백호참 캐스팅
function ON_CAST_1022(me, spell)
    local effect = 164
    local sound = 88
    local hp = me:hp()//2
    local mp = 50
    local damage = me:hp()

    local targets = {}
    local x, y = me:position()
    local direction = me:direction()

    local positions = {}
    if direction == DIRECTION_LEFT then
        table.insert(positions, {x-1, y  })
        table.insert(positions, {x-2, y-1})
        table.insert(positions, {x-2, y  })
        table.insert(positions, {x-2, y+1})
        table.insert(positions, {x-3, y-2})
        table.insert(positions, {x-3, y-1})
        table.insert(positions, {x-3, y  })
        table.insert(positions, {x-3, y+1})
        table.insert(positions, {x-3, y+2})
    elseif direction == DIRECTION_TOP then
        table.insert(positions, {x  , y-1})
        table.insert(positions, {x-1, y-2})
        table.insert(positions, {x  , y-2})
        table.insert(positions, {x+1, y-2})
        table.insert(positions, {x-2, y-3})
        table.insert(positions, {x-1, y-3})
        table.insert(positions, {x  , y-3})
        table.insert(positions, {x+1, y-3})
        table.insert(positions, {x+2, y-3})
    elseif direction == DIRECTION_RIGHT then
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
    for _, obj in pairs(me:nears(OBJECT_TYPE_LIFE)) do
        local obj_x, obj_y = obj:position()
        for _, position in pairs(positions) do
            if obj_x == position[1] and obj_y == position[2] then
                table.insert(targets, obj)
                break
            end
        end
    end

    if not attack_cast(me, targets, spell, hp, mp, damage, '극\'백호참', sound, effect) then
        return
    end
end