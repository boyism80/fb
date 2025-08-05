-- 삽사리 사망
function ON_MOB_DIE_4(me, you)
    local quest = me:quest(1)
    if quest ~= nil and quest:step() == 2 then
        local done = quest:inc_progress(1)
        if done then
            me:chat('I can complete current quest.')
        end
    end
end