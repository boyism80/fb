-- item: 백세주

-- 백세주 사용 스크립트
function ON_ACTIVATED_2204(me, item)
    me:mp(math.min(me:mp()+150, me:maxmp()))
end
