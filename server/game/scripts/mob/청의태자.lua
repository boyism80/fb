local quest = require('lib.quest')

function ON_MOB_DIE_1095(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = you:quest(quest.QUEST_DRAGON_KING)
    if q == nil then
        return
    end

    local step = q:step()
    if step == 18 then
        local btn = you:dialog(me, "헉헉. 이게 어떻게 된 노릇이냐.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = you:list(me, "나의 완벽했던 계획이 도대체 어디서부터 어그러졌단 말인가...", { "완벽한 계획?" }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
            return
        end
        sel, list_btn = you:list(me, " ", { "그런 일이 과연 성공할 것이라고 믿었는가?" }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
            return
        end
        btn = you:dialog(me, "이대로 죽을수는 없다. 아직 나에게는 여의주가 남아있다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = you:dialog(me, "자. 여의주여. 나에게 강한 힘을 달라. 나를 보호하라.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if not you:has_items("여의주", 1) then
            you:mkitem("여의주", 1)
            you:push_achievement(19, "청의태자를 생포하였다. [1/2]", 7, 0)
        end
        q:progress(1)
        return
    end

    if step ~= 19 then
        return
    end
    local sel, list_btn = you:list(me, "으윽. 이계로까지 도망갔다 온 나를 어떻게 발견했지...", { "네놈이 아무리 날고 뛰어봐야 용왕님이" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    sel, list_btn = you:list(me, " ", { "너에 대해 모든걸 알고 계신다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    local btn = you:dialog(me, "이대로 죽을수는 없다. 아직 나에게는 여의주가 남아있다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = you:dialog(me, "자. 여의주여. 나에게 강한 힘을 달라. 나를 보호하라.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    sel, list_btn = you:list(me, "으윽, 으아아아악.", { "헛. 이럴수가!!!" }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return
    end
    q:progress(2)
    you:push_achievement(19, "청의태자를 생포하였다. [2/2]", 7, 0)
    you:dialog(me, "아. 나의 꿈이....나의 야망이................", false, true)
end

-- 청의태자 공격
function ON_MOB_ATTACK_1095(me, you)
    math.randomseed(seed())
    if math.random() > 0.01 then
        return
    end
    
    if you == nil then
        return
    end

    me:chat('여의주의 힘을 받은 용이여..')
    sleep(1000);
    
    if not assert_alive(me) then
        return
    end
    me:chat('그대 이름은 청룡일지다...')
    sleep(1000);
    
    if not assert_alive(me) then
        return
    end
    me:chat('나 청의태자의 이름으로 말하노니...')
    sleep(1000);

    if not assert_alive(me) then
        return
    end
    me:chat('네 포효를 적에게 발산하라!!')
    if assert_alive(you) and me:near(you) then
        me:cast(you, '청룡마령참')
    end

    sleep(1000);
    me:chat('청룡마령참!!', 0x03)
    
    sleep(1000);
end