-- Shared helpers for 삼짇날 butterfly catching jars (채집통 [0..10 마리]).

local JAR_NAMES = {}
for i = 0, 10 do
    JAR_NAMES[i] = string.format("채집통 [%d 마리]", i)
end

local M = {}

function M.find_count(me)
    for i = 10, 0, -1 do
        if me:has_items(JAR_NAMES[i], 1) then
            return i
        end
    end
    return nil
end

function M.upgrade(me)
    local count = M.find_count(me)
    if count == nil then
        return false
    end
    if count >= 10 then
        me:message("채집통이 꽉 찼습니다.")
        return false
    end

    local from_name = JAR_NAMES[count]
    local to_name = JAR_NAMES[count + 1]
    if not me:rmitem(from_name, 1, ITEM_DELETE_TYPE.REDUCE) then
        return false
    end
    if me:mkitem(to_name, 1) == nil then
        me:mkitem(from_name, 1)
        return false
    end
    return true
end

function M.shake(me, item)
    local has_butterflies = false
    for i = 1, 10 do
        if me:has_items(JAR_NAMES[i], 1) then
            has_butterflies = true
            break
        end
    end
    if not has_butterflies then
        return
    end

    local model = item ~= nil and item:model() or me
    local btn = me:dialog(model, "앗... 채집통을 흔들었더니 나비가 모두 날아가 버렸다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    for i = 1, 10 do
        if me:has_items(JAR_NAMES[i], 1) then
            me:rmitem(JAR_NAMES[i], 1, ITEM_DELETE_TYPE.REDUCE)
        end
    end
    if not me:has_items(JAR_NAMES[0], 1) then
        me:mkitem(JAR_NAMES[0], 1)
    end
end

return M
