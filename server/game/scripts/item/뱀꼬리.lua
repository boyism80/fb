-- 뱀꼬리 사용 스크립트
function ON_ACTIVATED_3040(me, item)
    me:hp(math.min(me:hp()+78, me:maxhp()))
end