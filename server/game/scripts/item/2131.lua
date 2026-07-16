-- item: 시력회복의귀걸이

-- 시력회복의귀걸이 사용 스크립트
-- function ON_ACTIVATED_2131(me, item)
--
-- end

-- 시력회복의귀걸이 해제 스크립트
-- function ON_DEACTIVATED_2131(me, item)
--
-- end

-- 시력회복의귀걸이 지속 스크립트
function ON_CONCAST_2131(me, item)
    if me:isbuff('절망') then
        me:unbuff('절망')
    end
end
