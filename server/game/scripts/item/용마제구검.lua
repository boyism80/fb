-- 용마제구검: on attack, chance to cast 용마제구격 on front target. Trash: 50% probability.
function ON_ATTACK_257(me, item)
    math.randomseed(seed())
    if math.random() > 0.50 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용마제구격')
end

-- 용마제구검 사용 스크립트
function ON_ACTIVATED_257(me, item)

end

-- 용마제구검 해제 스크립트
function ON_INACTIVE_257(me, item)

end