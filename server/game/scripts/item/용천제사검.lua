-- 용천제사검 공격 스크립트
function ON_ATTACK_261(me, item)
    math.randomseed(seed())
    if math.random() > 0.35 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용천제사격')
end

-- 용천제사검 사용 스크립트
function ON_ACTIVATED_261(me, item)

end

-- 용천제사검 해제 스크립트
function ON_DEACTIVATED_261(me, item)

end