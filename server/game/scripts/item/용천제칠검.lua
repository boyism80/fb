-- 용천제칠검: on attack, chance to cast 용천제칠격 on front target. Trash: 50% probability.
function ON_ATTACK_264(me, item)
    math.randomseed(seed())
    if math.random() > 0.50 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용천제칠격')
end

-- 용천제칠검 사용 스크립트
function ON_ACTIVE_264(me, item)

end

-- 용천제칠검 해제 스크립트
function ON_INACTIVE_264(me, item)

end