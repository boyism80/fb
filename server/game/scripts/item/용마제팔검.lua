-- 용마제팔검 공격 스크립트
function ON_ATTACK_256(me, item)
    math.randomseed(seed())
    if math.random() > 0.50 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용마제팔격')
end

-- 용마제팔검 사용 스크립트
function ON_ACTIVATED_256(me, item)

end

-- 용마제팔검 해제 스크립트
function ON_INACTIVE_256(me, item)

end