-- 용천제육검: on attack, chance to cast 용천제육격 on front target. Trash: 45% probability.
function ON_ATTACK_263(me, item)
    math.randomseed(seed())
    if math.random() > 0.45 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용천제육격')
end

-- 용천제육검 사용 스크립트
function ON_ACTIVE_263(me, item)

end

-- 용천제육검 해제 스크립트
function ON_INACTIVE_263(me, item)

end