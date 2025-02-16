function npc_revive(me, npc, discourteous)
    if me:state() ~= STATE_GHOST then
        return true
    end

    if discourteous then
        npc:chat('곱게 빌어도 살려줄까 말까 하는데...')
        return false
    end

    local i = math.random(0, 5)
    if i == 0 then
        npc:chat('생명의 소중함을 그렇게 가르쳤거늘...')
    elseif i == 1 then
        npc:chat('생즉시공 공즉시생')
    elseif i == 2 then
        npc:chat('인생의 뜻을 모르고 있구나..')
    elseif i == 3 then
        npc:chat('정성이 부족하구나...')
    elseif i == 4 then
        npc:chat('그런 마음가짐으로 생명을 얻겠느냐?')
    else
        npc:chat('너의 정성에 감복하여 살려줄터이니 잠시 기다려라....')
        sleep(1000)
        npc:chat('영은 생을 얻을지어니...')
        me:state(STATE_NORMAL)
        me:hp(50)
    end
    return true
end

function npc_appreciate(me, npc)
    if me:hp() > 100 then
        me:chat(me:hp())
        return true
    end

    npc:chat('예의 바른 놈. 잘 살거라.')
    me:hp(me:maxhp())
    return true
end