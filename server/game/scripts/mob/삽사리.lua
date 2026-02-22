-- 삽사리 사망
function ON_MOB_DIE_4(me, you)
    local quest = you:quest(QUEST_NAKRANG_HUNT)
    if quest ~= nil then
        if me:model():name() == quest:param() then
            quest:inc_progress()
        end
    end
end