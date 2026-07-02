-- item: 자호의가죽

-- 자호의가죽 사용 스크립트
function ON_ACTIVATED_3028(me, item)
    me:hp(math.min(me:hp()+18, me:maxhp()))
end
