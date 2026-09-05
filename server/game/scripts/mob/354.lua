-- mob: 고급유령
local quest = require('lib.quest')

return {
    on_mob_attack = function(me, you)
        if math.random(5) ~= 1 then
            return
        end
        local lines = {
            '겔겔겔겔...',
            '에잇...',
            '나의 빠른 칼을 받아랏!',
        }
        me:chat(lines[math.random(#lines)])
    end,

    on_mob_kill = function(me, mobs)
        quest.king_on_mob_kill(me, mobs)
    end,
}
