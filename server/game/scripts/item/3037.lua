-- item: 사슴고기

-- 사슴고기 사용 스크립트
function ON_ACTIVATED_3037(me, item)
    me:hp(math.min(me:hp()+20, me:maxhp()))
end
