-- item: 용마제사검

-- 용마제사검 공격 스크립트
function ON_ATTACK_252(me, item)
    math.randomseed(seed())
    if math.random() > 0.35 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    me:cast(front, '용마제사격')
end

-- 용마제사검 사용 스크립트
-- function ON_ACTIVATED_252(me, item)
--
-- end

-- 용마제사검 해제 스크립트
-- function ON_DEACTIVATED_252(me, item)
--
-- end
