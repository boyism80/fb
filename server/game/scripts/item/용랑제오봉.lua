-- 용랑제오봉: on attack, chance to apply 용의제오주 on front target. Trash: 25% prob, 6s.
function ON_ATTACK_271(me, item)
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
    front:buff('용의제오주', 6, me)
end

-- 용랑제오봉 사용 스크립트
function ON_ACTIVATED_271(me, item)

end

-- 용랑제오봉 해제 스크립트
function ON_INACTIVE_271(me, item)

end