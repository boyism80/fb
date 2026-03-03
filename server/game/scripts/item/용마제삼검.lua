-- 용마제삼검: on attack, chance to cast 용마제삼격 on front target. Trash: 30% probability.
function ON_ATTACK_251(me, item)
    math.randomseed(seed())
    if math.random() > 0.30 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용마제삼격')
end

-- 용마제삼검 사용 스크립트
function ON_ACTIVE_251(me, item)

end

-- 용마제삼검 해제 스크립트
function ON_INACTIVE_251(me, item)

end