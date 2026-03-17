-- 낡은봉투 사용 스크립트
function ON_ACTIVATED_10248(me, item)
    local model = item:model()
    local selected = me:list(model, '봉투를 여시겠습니까?', {'열지 않겠어. 약속은 지켜야해.', '열어봐야지. 약속보다는 궁금함이 먼저야.'})
    if selected == nil then
        return
    end

    if selected == 0 then
        return
    end

    me:rmitem(item)
    local added = me:mkitem('낡은연애편지')
    me:active(added)
end