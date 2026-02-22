function ON_MOB_DIE_1081(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local quest = you:quest(QUEST_DRAGON_KING)
    if quest == nil or quest:step() ~= 3 then
        return
    end

    local sel, list_btn = you:list(me, "으...분하다...네놈에게 지다니...", { "이놈, 나의 포박을 받아라", "용왕님이 너를 보자하신다.", "따라오너라" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 2 then
        return
    end

    quest:progress(1)
    you:push_achievement(19, "게장군을 생포했다!", 7, 0)
end

-- 게장군 공격
function ON_MOB_ATTACK_1081(me, you)

end