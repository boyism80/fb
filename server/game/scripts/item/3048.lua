-- item: 구운고구마

-- 구운고구마 사용 스크립트
function ON_ACTIVATED_3048(me, item)
    me:hp(math.min(me:hp()+150, me:maxhp()))
end
