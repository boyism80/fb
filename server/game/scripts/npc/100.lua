-- npc: 장안황제
local quest = require('lib.quest')

return {
    on_click = function(me, npc)
        quest.king_dialog(me, name2npc('장안성경비대장') or npc, {
            quest_id = quest.QUEST_KING_JANGAN,
            min_level = 99,
            achievement_id = 43,
            curse_sec = 600,
            king_name = '장안황제',
        })
    end
}
