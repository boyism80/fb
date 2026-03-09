-- 용천제일검 공격 스크립트
function ON_ATTACK_258(me, item)
    math.randomseed(seed())
    if math.random() > 0.20 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용천제일격')
end

-- 용천제일검 사용 스크립트
function ON_ACTIVATED_258(me, item)

end

-- 용천제일검 해제 스크립트
function ON_INACTIVE_258(me, item)

end