-- mob: 전갈장
local quest = require('lib.quest')

return {
    on_mob_attack = function(me, you)
        if math.random(5) ~= 1 then
            return
        end
        me:chat('뀌익 끼익 칙칙')
    end,

    on_mob_kill = function(me, mobs)
        quest.king_on_mob_kill(me, mobs)
    end,
}
