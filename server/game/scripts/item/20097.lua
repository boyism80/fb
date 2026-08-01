-- item: 낡은금고

local enum = require('lib.enum')

return {
    on_activated = function(me, item)
    	local model = item:model()
    	if me:item('무쇠망치') == nil then
    		me:dialog(model, '자세히 살펴보니, 무쇠망치가 있으면 부술 수 있을 것 같다.')
    		return
    	end

    	local code = me:exchange(
    		{ ['item'] = { ['낡은금고'] = 1, ['무쇠망치'] = 1 } },
    		{ ['item'] = { ['낡은봉투'] = 1 } }
    	)
    	if code == enum.exchange_result.LACK_CAPACITY then
    		me:dialog(model, '소지품이 가득 찼습니다.')
    		return
    	end
    	me:dialog(model, '무쇠망치로 낡은 금고를 깨뜨리니, 안에서 낡은 봉투가 나왔군.')
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
