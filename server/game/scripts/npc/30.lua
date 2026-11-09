-- npc: 고구려왕
local quest = require('lib.quest')

return {
    on_click = function(me, npc)
        quest.king_dialog(me, name2npc('고구려경비대장') or npc, {
            quest_id = quest.QUEST_KING_GOGURYEO,
            min_level = 56,
            nation = NATION.GOGURYEO,
            nation_reject_msg = '여기는 고구려왕이 계시는 곳입니다.',
            achievement_id = 8,
            curse_sec = 430,
            king_name = '고구려왕',
        })
    end
}
