-- item: 용랑제구봉

-- 용랑제구봉 공격 스크립트
function ON_ATTACK_275(me, item)
    math.randomseed(seed())
    if math.random() > 0.65 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    front:sound(40)
    front:effect(53)
    front:buff('용의제구주', 15, me)
end

-- 용랑제구봉 사용 스크립트
function ON_ACTIVATED_275(me, item)

end

-- 용랑제구봉 해제 스크립트
function ON_DEACTIVATED_275(me, item)

end
