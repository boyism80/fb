-- 토끼 사망
function ON_MOB_DIE_2(me, you)
    local quest = you:quest(1)
    if quest ~= nil then
        if me:model():name() == quest:param() then
            quest:inc_progress()
        end
    end
end