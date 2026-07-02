-- item: 4차도시락

-- 4차도시락 사용 스크립트
function ON_ACTIVATED_3090(me, item)
    me:hp(math.min(me:hp()+100, me:maxhp()))
end
