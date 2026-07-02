-- item: 닭고기

-- 닭고기 사용 스크립트
function ON_ACTIVATED_3027(me, item)
    me:hp(math.min(me:hp()+78, me:maxhp()))
end
