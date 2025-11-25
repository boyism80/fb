-- 월아검 공격 스크립트
function ON_ATTACK_69(me, item)
    math.randomseed(seed())
    if math.random() > 0.3 then
        return false
    end

    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end

    me:cast(front, '월아일격')
end

-- 월아검 사용 스크립트
function ON_ACTIVE_69(me, item)

end

-- 월아검 해제 스크립트
function ON_INACTIVE_69(me, item)

end