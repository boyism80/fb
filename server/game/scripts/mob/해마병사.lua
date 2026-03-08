function ON_MOB_DIE_1085(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local quest = you:quest(QUEST_DRAGON_KING)
    if quest == nil or quest:step() ~= 7 then
        return
    end

    local p = quest:progress()
    if p < 500 then
        quest:progress(p + 1)
    end
end