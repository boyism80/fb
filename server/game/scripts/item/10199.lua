-- item: 표신궁고문서5

-- 표신궁고문서5 사용 스크립트
function ON_ACTIVATED_10199(me, item)
    local model = item:model()
    me:dialog(model, '알아볼 수 없는 문자로 쓰여 있어 읽을 수 없다.')
end
