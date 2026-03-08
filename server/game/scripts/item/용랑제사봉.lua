-- 용랑제사봉: on attack, chance to apply 용의제사주 on front target. Trash: 25% prob, 6s.
function ON_ATTACK_270(me, item)
    math.randomseed(seed())
    if math.random() > 0.25 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    front:sound(40)
    front:effect(53)
    front:buff('용의제사주', 6, me)
end

-- 용랑제사봉 사용 스크립트
function ON_ACTIVE_270(me, item)

end

-- 용랑제사봉 해제 스크립트
function ON_INACTIVE_270(me, item)

end