-- 용천제삼검 공격 스크립트
function ON_ATTACK_260(me, item)
    math.randomseed(seed())
    if math.random() > 0.30 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용천제삼격')
end

-- 용천제삼검 사용 스크립트
function ON_ACTIVATED_260(me, item)

end

-- 용천제삼검 해제 스크립트
function ON_DEACTIVATED_260(me, item)

end