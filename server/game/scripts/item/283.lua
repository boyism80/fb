-- item: 용겸제팔봉
-- 용겸제팔봉 공격 스크립트
function ON_ATTACK_283(me, item)
    math.randomseed(seed())
    if math.random() > 0.50 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    front:sound(40)
    front:effect(53)
    front:buff('용의제팔노', 13, me)
end

-- 용겸제팔봉 사용 스크립트
function ON_ACTIVATED_283(me, item)

end

-- 용겸제팔봉 해제 스크립트
function ON_DEACTIVATED_283(me, item)

end
