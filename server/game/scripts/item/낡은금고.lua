-- 낡은금고 사용 스크립트
function ON_ACTIVATED_20097(me, item)
	local model = item:model()
	if me:item('무쇠망치') == nil then
		me:dialog(model, '자세히 살펴보니, 무쇠망치가 있으면 부술 수 있을 것 같다.')
		return
	end

	me:rmitem('무쇠망치')
	me:rmitem('낡은금고')
	me:mkitem('낡은봉투')
	me:dialog(model, '무쇠망치로 낡은 금고를 깨뜨리니, 안에서 낡은 봉투가 나왔군.')
end