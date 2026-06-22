-- item: 용천제이검
-- 용천제이검 공격 스크립트
function ON_ATTACK_259(me, item)
    math.randomseed(seed())
    if math.random() > 0.25 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용천제이격')
end

-- 용천제이검 사용 스크립트
function ON_ACTIVATED_259(me, item)

end

-- 용천제이검 해제 스크립트
function ON_DEACTIVATED_259(me, item)

end
