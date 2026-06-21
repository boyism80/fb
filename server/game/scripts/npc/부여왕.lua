local quest = require('lib.quest')

function NPC_31(me, npc)
    local function pool_fn(level)
        if level < 70 then
            return { '자호', '친자호', '구자호', '가재', '전갈', '전갈장', '가재장', '서현가재' }
        elseif level < 90 then
            return { '유령', '초급유령', '중급유령', '고급유령', '자령', '주령', '지령' }
        else
            return { '처녀귀신', '달걀귀신', '불귀신' }
        end
    end
    quest.king_dialog(me, name2npc('부여경비대장') or npc, {
        quest_id = quest.QUEST_KING_BUYEO,
        min_level = 56,
        nation = NATION.BUYEO,
        nation_reject_msg = '여기는 부여왕이 계시는 곳입니다.',
        achievement_id = 8,
        curse_sec = 430,
        king_name = '부여왕',
        pool_fn = pool_fn,
    })
end