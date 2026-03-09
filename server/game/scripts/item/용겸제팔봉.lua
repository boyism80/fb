-- 용겸제팔봉: on attack, chance to apply 용의제팔노 on front target. Trash: 50% prob, 13s.
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
function ON_INACTIVE_283(me, item)

end