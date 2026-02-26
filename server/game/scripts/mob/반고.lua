-- 반고 공격
function ON_MOB_ATTACK_1162(me, you)
    return false
end

-- 반고 사망
function ON_MOB_DIE_1162(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    local q = you:quest(QUEST_MUTA)
    if q == nil or q:step() ~= 6 then
        return
    end
    you:mkitem('반고의심장', 1)
end