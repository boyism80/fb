-- 이백세주 사용 스크립트
function ON_ACTIVATED_2206(me, item)
    me:mp(math.min(me:mp()+150, me:maxmp()))
end