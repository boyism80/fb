-- mob: 순제

local quest = require('lib.quest')

return {
    on_mob_action = function(me, you)
        return false
    end,


    on_mob_die = function(me)
        me:chat('이제야... 모든...준비가...', CHAT_TYPE.NORMAL, true)
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
        if q == nil or q:step() ~= 5 then
            return
        end
        if me:mkitem('순제의증표', 1) == nil then
            me:dialog(mob, '소지품이 가득 차서 순제의증표를 받을 수 없습니다.', { prev = false, next = false })
        end
    end

    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end,
}
