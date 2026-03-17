-- 용랑제일봉 공격 스크립트
function ON_ATTACK_267(me, item)
    math.randomseed(seed())
    if math.random() > 0.15 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    front:sound(40)
    front:effect(53)
    front:buff('용의제일주', 6, me)
end

-- 용랑제일봉 사용 스크립트
function ON_ACTIVATED_267(me, item)

end

-- 용랑제일봉 해제 스크립트
function ON_DEACTIVATED_267(me, item)

end