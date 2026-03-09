-- 용겸제육봉: on attack, chance to apply 용의제육노 on front target. Trash: 25% prob, 21s.
function ON_ATTACK_281(me, item)
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
    front:buff('용의제육노', 21, me)
end

-- 용겸제육봉 사용 스크립트
function ON_ACTIVATED_281(me, item)

end

-- 용겸제육봉 해제 스크립트
function ON_INACTIVE_281(me, item)

end