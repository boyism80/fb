-- item: 대장군의일기3

return {
    on_activated = function(me, item)
        local model = item:model()
        local button = nil

    ::ON_ACTIVATED_10203_001::
    	button = me:dialog(model, '-궐기한지 31일째\n예감은 적중했다.\n천황은 남은 영지에 밀사를 보내 연합군을 조직, 우리를 사방에서 포위하고 공격해왔다.\n천황의 지독한 폭정에 모두 반발하고 있었으니, 천황의 명을 따르지 않을 것이라고 생각했건만. 분명 많은 부와 더 좋은 지위를 미끼로 유혹했겠지...', { prev = false, next = true })
    	if button == DIALOG_RESULT.QUIT then
    		return
    	end
    ::ON_ACTIVATED_10203_002::
    	button = me:dialog(model, '일기를 쓰고 있을 시간이 없다.\n오늘 밤에 있을 전투가 우리 군의 미래를 결정할 것이다.', { prev = true, next = false })
    	if button == DIALOG_RESULT.PREV then
    		goto ON_ACTIVATED_10203_001
    	end
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
