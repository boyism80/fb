-- item: 홍주

-- 홍주 사용 스크립트
function ON_ACTIVATED_2205(me, item)
    me:mp(math.min(me:mp()+300, me:maxmp()))
end
