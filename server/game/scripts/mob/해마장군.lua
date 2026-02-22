function ON_MOB_DIE_1086(me, you)
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

    quest:param("1")
    you:push_achievement(19, "해마장군을 생포하였다!", 7, 0)
end

-- 해마장군 공격
function ON_MOB_ATTACK_1086(me, you)

end