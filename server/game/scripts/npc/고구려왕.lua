local quest = require('lib.quest')

function NPC_30(me, npc)
    local function pool_fn(level)
        if level < 70 then
            return { '자호', '친자호', '구자호', '사마귀', '거미', '사마귀랑', '거미랑', '서현거미' }
        elseif level < 90 then
            return { '해골', '칼든해골', '날쌘해골', '쾌도해골', '자해골', '주해골', '지해골' }
        else
            return { '몽달귀신', '독충', '빗자루귀신' }
        end
    end
    quest.king_dialog(me, name2npc('고구려경비대장') or npc, {
        quest_id = quest.QUEST_KING_GOGURYEO,
        min_level = 56,
        nation = NATION.GOGURYEO,
        nation_reject_msg = '여기는 고구려왕이 계시는 곳입니다.',
        achievement_id = 8,
        curse_sec = 430,
        king_name = '고구려왕',
        pool_fn = pool_fn,
    })
end