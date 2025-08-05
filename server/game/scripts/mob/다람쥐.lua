-- 다람쥐 사망
function ON_MOB_DIE_1(me, you)
    if you ~= nil then
        local quest = you:quest(1)
        if quest ~= nil and quest:step() == 0 then
            local done = quest:inc_progress(1)
            if done then
                you:chat('I can complete current quest.')
            end
        end
    end
end