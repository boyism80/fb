-- 파란열쇠 사용 스크립트
function ON_ACTIVE_20211(me, item)
    local map = me:map()
    local door = map:door(me)
    if door == nil then
        return
    end

    local locked = door:lock(not door:locked())
    if locked then
        me:message('문을 잠궜습니다.')
    else
        me:message('문을 열었습니다.')
    end
end