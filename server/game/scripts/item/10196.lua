-- item: 표신궁고문서2

return {
    ON_ACTIVATED = function(me, item)
        local model = item:model()
        me:dialog(model, '알아볼 수 없는 문자로 쓰여 있어 읽을 수 없다.')
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
