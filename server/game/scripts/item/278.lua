-- item: 용겸제삼봉
-- 용겸제삼봉 공격 스크립트
function ON_ATTACK_278(me, item)
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
    front:buff('용의제삼노', 6, me)
end

-- 용겸제삼봉 사용 스크립트
function ON_ACTIVATED_278(me, item)

end

-- 용겸제삼봉 해제 스크립트
function ON_DEACTIVATED_278(me, item)

end
