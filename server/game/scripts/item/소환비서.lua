-- 소환비서 사용 스크립트
function ON_ACTIVE_4001(me, item)
    local front = me:front(OBJECT_TYPE.CHARACTER)
    if front ~= nil then
        if front:level() < me:level() then
            local x, y = me:position()
            front:position(x, y)
        end
    end

    me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
end