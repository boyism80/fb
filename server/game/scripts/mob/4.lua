-- mob: 삽사리
-- 삽사리 사망
local quest = require('lib.quest')

function ON_MOB_DIE_4(me, you)
    local q = you:quest(quest.QUEST_NAKRANG_HUNT)
    if q ~= nil then
        if me:model():name() == q:param() then
            q:inc_progress()
        end
    end
end
