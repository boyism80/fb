-- mob: 상어장군
local quest = require('lib.quest')

return {
    -- on_mob_attack = function(me, you)
    -- end,

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
        if step == 12 and q:progress() == 0 then
            local sel, list_btn = me:list(mob, "으윽... 비록 나는 잡힐지언정 우리 군사들의 사기는 절대 꺽이지 않을 것이다.", { "닥쳐라", "어서 나를 따라서 용왕님을 뵈러 가자." }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 1 or sel > 2 then
                return
            end
            q:progress(1)
            me:push_achievement(19, "상어장군을 생포하였다. [1/2]", 7, 1)
            return
        end

        if step ~= 13 then
            return
        end

        local anger_end = tonumber(q:param() or "")
        local t = now()
        if not anger_end or t < anger_end then
            return
        end

        local sel, list_btn = me:list(mob, "한번만 눈감아 주시면 평생 쓸만한 금은보화를 드리겠습니다.", { "으음...그래 좋다.", "나는 금은보화 따위에 넘어가지 않는다!" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            q:progress(2)
            me:mkitem("금은보화", 1)
            me:push_achievement(19, "상어장군을 놓아주다.", 7, 1)
            me:dialog(mob, "감사합니다. 자 이걸 받아주십시오. 그럼 전 사라지겠습니다.", { prev = false, next = true })
        else
            q:progress(1)
            me:push_achievement(19, "상어장군을 생포하였다. [2/2]", 7, 1)
        end
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
