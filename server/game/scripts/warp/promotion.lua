local door = require('lib.promotion_door')

return {
    on_east_taegeuk = function(me)
        door.key_door(me, '동문열쇠', '동문열쇠가 필요합니다.', 1772, 13, 13)
    end,

    on_west_taegeuk = function(me)
        door.key_door(me, '서문열쇠', '서문열쇠가 필요합니다.', 1773, 13, 13)
    end,

    on_soho = function(me)
        door.boss_door(me, { exact = 1, map = 2009, x1 = 11, x2 = 18, y1 = 28 })
    end,

    on_jeonuk = function(me)
        door.boss_door(me, { exact = 2, map = 2008, x1 = 12, x2 = 17, y1 = 2 })
    end,

    on_jegok = function(me)
        door.boss_door(me, { exact = 3, map = 1964, x1 = 2, y1 = 12, y2 = 17 })
    end,

    on_yoje = function(me)
        door.boss_door(me, { exact = 4, map = 2007, x1 = 27, y1 = 12, y2 = 17 })
    end,

    on_sunje = function(me)
        door.boss_door(me, { minimum = 5, map = 2006, x1 = 12, x2 = 17, y1 = 27 })
    end,

    on_bango = function(me)
        door.boss_door(me, { minimum = 6, map = 2010, x1 = 9, x2 = 16, y1 = 23 })
    end,

    on_confirm = function(me, leader_name)
        local selected, button = me:list(me, string.format('%s의 동료로 같이 들어가겠느냐?', leader_name), { '네.', '아니오.' })
        if button == DIALOG_RESULT.QUIT or selected ~= 1 then
            return false
        end
        return true
    end,
}
