-- 용랑제칠봉: on attack, chance to apply 용의제칠주 on front target. Trash: 40% prob, 12s.
function ON_ATTACK_273(me, item)
    math.randomseed(seed())
    if math.random() > 0.40 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    front:sound(40)
    front:effect(53)
    front:buff('용의제칠주', 12, me)
end

-- 용랑제칠봉 사용 스크립트
function ON_ACTIVE_273(me, item)

end

-- 용랑제칠봉 해제 스크립트
function ON_INACTIVE_273(me, item)

end