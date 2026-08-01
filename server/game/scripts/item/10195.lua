-- item: 표신궁고문서1

return {
    on_activated = function(me, item)
        local model = item:model()
        me:dialog(model, '알아볼 수 없는 문자로 쓰여 있어 읽을 수 없다.')
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
