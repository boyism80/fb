-- npc: 부여왕
local quest = require('lib.quest')

return {
    on_click = function(me, npc)
        quest.king_dialog(me, name2npc('부여경비대장') or npc, {
            quest_id = quest.QUEST_KING_BUYEO,
            min_level = 56,
            nation = NATION.BUYEO,
            nation_reject_msg = '여기는 부여왕이 계시는 곳입니다.',
            achievement_id = 8,
            curse_sec = 430,
            king_name = '부여왕',
        })
    end
}
