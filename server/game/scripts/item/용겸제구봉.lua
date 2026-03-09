-- 용겸제구봉: on attack, chance to apply 용의제구노 on front target. Trash: 65% prob, 15s.
function ON_ATTACK_284(me, item)
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
    front:buff('용의제구노', 15, me)
end

-- 용겸제구봉 사용 스크립트
function ON_ACTIVATED_284(me, item)

end

-- 용겸제구봉 해제 스크립트
function ON_INACTIVE_284(me, item)

end