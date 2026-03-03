-- 용마제오검: on attack, chance to cast 용마제오격 on front target. Trash: 40% probability.
function ON_ATTACK_253(me, item)
    math.randomseed(seed())
    if math.random() > 0.40 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용마제오격')
end

-- 용마제오검 사용 스크립트
function ON_ACTIVE_253(me, item)

end

-- 용마제오검 해제 스크립트
function ON_INACTIVE_253(me, item)

end