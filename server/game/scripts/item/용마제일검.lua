-- 용마제일검: on attack, chance to cast 용마제일격 on front target. Trash: 20% probability.
function ON_ATTACK_249(me, item)
    math.randomseed(seed())
    if math.random() > 0.20 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용마제일격')
end

-- 용마제일검 사용 스크립트
function ON_ACTIVATED_249(me, item)

end

-- 용마제일검 해제 스크립트
function ON_INACTIVE_249(me, item)

end