-- mob: 청의태자
local quest = require('lib.quest')

return {
    on_mob_attack = function(me, you)
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
    end,

    -- on_mob_die = function(me)
    -- end,

    on_mob_kill = function(me, mobs)
        if me == nil or mobs == nil or #mobs == 0 then
            return
        end
        local mob = mobs[1]
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local q = me:quest(quest.QUEST_DRAGON_KING)
        if q == nil then
            return
        end

        local step = q:step()
        if step == 18 then
            local btn = me:dialog(mob, "헉헉. 이게 어떻게 된 노릇이냐.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local sel, list_btn = me:list(mob, "나의 완벽했던 계획이 도대체 어디서부터 어그러졌단 말인가...", { "완벽한 계획?" }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
                return
            end
            sel, list_btn = me:list(mob, " ", { "그런 일이 과연 성공할 것이라고 믿었는가?" }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
                return
            end
            btn = me:dialog(mob, "이대로 죽을수는 없다. 아직 나에게는 여의주가 남아있다.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(mob, "자. 여의주여. 나에게 강한 힘을 달라. 나를 보호하라.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if not me:has_items("여의주", 1) then
                me:mkitem("여의주", 1)
                me:push_achievement(19, "청의태자를 생포하였다. [1/2]", 7, 1)
            end
            q:progress(1)
            return
        end

        if step ~= 19 then
            return
        end
        local sel, list_btn = me:list(mob, "으윽. 이계로까지 도망갔다 온 나를 어떻게 발견했지...", { "네놈이 아무리 날고 뛰어봐야 용왕님이" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end
        sel, list_btn = me:list(mob, " ", { "너에 대해 모든걸 알고 계신다." }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end
        local btn = me:dialog(mob, "이대로 죽을수는 없다. 아직 나에게는 여의주가 남아있다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(mob, "자. 여의주여. 나에게 강한 힘을 달라. 나를 보호하라.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        sel, list_btn = me:list(mob, "으윽, 으아아아악.", { "헛. 이럴수가!!!" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
            return
        end
        q:progress(2)
        me:push_achievement(19, "청의태자를 생포하였다. [2/2]", 7, 1)
        me:dialog(mob, "아. 나의 꿈이....나의 야망이................", { prev = false, next = true })
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
