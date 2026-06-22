-- npc: 공성성황
function NPC_212(me, npc)
    if me:state() ~= STATE.GHOST then
        return me:dialog(npc, '사망 상태가 아닐 때 나오는 메시지')
    end

    if me:menu(npc, '생명의 소중함을 그렇게 일러왔거늘... 앞으로돋 사소한 일에 목숨을걸지 않으리라고 내가 어떻게 믿을 수 있겠느냐? 또 생명을 잃고 나를 찾아오지 않겠다고 맹세할 수 있겠느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    if me:menu(npc, '싸움은 싸움을 부르고, 피는 반드시 피를 보게 되느니라. 이번의 죽음도 네 책임이라는 것을 진심으로 느끼고 반성하고 있느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    if me:menu(npc, '그렇다면 잃은 물건과 경험치도 다 네 욕심에서 비롯되었음을 인정하겠느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    if me:menu(npc, '네가 새로 생명을 얻게 되더라도 절대로 무고한 생명을 해치지 않을 것을 맹세하느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    me:dialog(npc, '너의 각오를 믿고 새로운 생명을 내리노니 나에게 한 맹세를 잊지 말고 하루하루를 신께 감사하는 마음으로 살아가도록 하여라.')
    me:state(STATE.NORMAL)
    me:hp(50)
end
