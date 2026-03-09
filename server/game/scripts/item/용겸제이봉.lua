-- 용겸제이봉: on attack, chance to apply 용의제이노 on front target. Trash: 20% prob, 6s.
function ON_ATTACK_277(me, item)
    math.randomseed(seed())
    if math.random() > 0.20 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    front:sound(40)
    front:effect(53)
    front:buff('용의제이노', 6, me)
end

-- 용겸제이봉 사용 스크립트
function ON_ACTIVATED_277(me, item)

end

-- 용겸제이봉 해제 스크립트
function ON_INACTIVE_277(me, item)

end