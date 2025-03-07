-- 다람쥐 사망
function ON_MOB_DIE_1(me, you)
    me:chat('다람쥐 사망')

    local model = me:model()
    model:dialog(you, '나를 쓰러트리다니 대단하구나')
    model:dialog(you, '구라야')
end