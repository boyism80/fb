-- 토끼 사망
function ON_MOB_DIE_2(me, you)
    if you ~= nil then
        local quest = you:quest(1)
        you:chat('quest step: ' .. quest:step())
        if quest ~= nil and quest:step() == 1 then
            local done = quest:inc_progress(1)
            if done then
                you:chat('I can complete current quest.')
            end
        end
    end
end