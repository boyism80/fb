-- 다람쥐 사망
function ON_MOB_DIE_1(me, you)
    local success, size = you:group(group_lock)
    local npc = name2npc('낙랑')
    npc:dialog(you, string.format('%s, %s', success, size))

    npc:dialog(you, string.format('%s, %s', success, size))
end