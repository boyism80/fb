-- item: 호랑이가죽

-- 호랑이가죽 사용 스크립트
function ON_ACTIVATED_3024(me, item)
    me:hp(math.min(me:hp()+18, me:maxhp()))
end
