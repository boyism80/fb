-- 얼음칼 공격 스크립트
function ON_ATTACK_29(me, item)
    math.randomseed(seed())
    if math.random() > 0.3 then
        return false
    end

    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end

    if front:isbuff(table.unpack(relative_buff_name('마비'))) then
        return
    end

    front:sound(70)
    front:effect(1)
    front:buff('마비', 20, me)
end

-- 얼음칼 사용 스크립트
function ON_ACTIVATED_29(me, item)

end

-- 얼음칼 해제 스크립트
function ON_DEACTIVATED_29(me, item)

end