-- mob: 삽사리
-- 삽사리 사망
local quest = require('lib.quest')

function ON_MOB_KILL_4(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local q = me:quest(quest.QUEST_NAKRANG_HUNT)
    if q ~= nil then
        if mobs[1]:model():name() == q:param() then
            q:inc_progress(#mobs)
        end
    end
end

function ON_MOB_DIE_4(me)
end
