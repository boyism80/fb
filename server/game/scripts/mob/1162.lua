-- mob: 반고
local quest = require('lib.quest')

return {
    on_mob_attack = function(me, you)
        local lines = {
            '반고: 가소롭군..',
            '반고: 진흑룡소환!!',
        }
        me:chat(lines[math.random(#lines)], CHAT_TYPE.NORMAL, false)
        return false
    end,

    on_mob_die = function(me)
        me:chat('이 순간이..너희들의 기억속에서..지워지는 날..다시 오리라...', CHAT_TYPE.NORMAL, true)
    end,

    on_mob_kill = function(me, mobs)
        if me == nil or mobs == nil or #mobs == 0 then
            return
        end
        local mob = mobs[1]
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local q = me:quest(quest.QUEST_MUTA)
        if q == nil or q:step() ~= 6 then
            return
        end
        me:mkitem('반고의심장', 1)
    end
}
