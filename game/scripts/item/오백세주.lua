-- 오백세주 사용 스크립트
function ON_ACTIVE_2208(me, item)
    me:mp(math.min(me:mp()+150, me:maxmp()))
end