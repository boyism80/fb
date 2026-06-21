local quest = require('lib.quest')

function NPC_100(me, npc)
    local function pool_fn(level)
        if level < 71 then
            return { '녹살쾡이', '표황살쾡이', '외눈황천구', '외눈자천구', '흑혈후', '녹혈후', '흑선문후', '흡혈강시', '아기강시', '흑발강시' }
        elseif level < 86 then
            return { '선비평민', '선비검객', '흡혈강시', '아기강시', '흑발강시' }
        else
            return { '선비평민', '선비검객', '흉노평민', '흉노무사', '흉노전사' }
        end
    end
    quest.king_dialog(me, name2npc('장안성경비대장') or npc, {
        quest_id = quest.QUEST_KING_JANGAN,
        min_level = 99,
        achievement_id = 43,
        curse_sec = 600,
        king_name = '장안황제',
        pool_fn = pool_fn,
    })
end