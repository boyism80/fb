-- 생명보존반찬 사용 스크립트
function ON_ACTIVATED_3007(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end