-- map: 달맞이다리

return {
    on_bridge_buff = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        me:buff('다리밟기', 600)
        me:message('다리밟기 외웠습니다.', MESSAGE_TYPE.NOTIFY)
    end,
}
