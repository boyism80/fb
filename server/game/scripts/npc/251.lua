-- npc: 철수나한
function NPC_251(me, npc)
    local has_spell = (me:spell("금강퇴") ~= nil)

    local function run_welcome_and_list()
        local msg
        if has_spell then
            msg = "잘왔소. 금강퇴는 바로 앞의 적에게 강력한 완력으로 치명타를 입히는 무공이오.\n\n단 금강퇴를 전수받기 위해서는 소림사에 십만전을 시주해야 하오. 돈은 소림사의 발전을 위해 쓰여진다오."
        else
            msg = "소림사에 오신걸 환영하오. 난 소림십팔나한중의 한명인 철수나한이오. 소림사의 무공중에서도 십팔나한의 무공은 최고라 할 수 있소.\n\n그중에 나의 특기인 금강퇴는 완력을 사용하는 무공으로는 최고라오"
        end
        local button = me:dialog(npc, msg, { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return nil, true
        end
        local sel, btn = me:list(npc, "무슨일로 나를 찾아오셨소?", {
            "바로 그 금강퇴를 배우고 싶습니다.",
            "금강퇴의 수련을 위해 찾아왔습니다.",
            "금강퇴의 수련방법에 대해서 좀더 알고싶습니다.",
        }, { prev = false })
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return nil, true
        end
        return sel, false
    end

    local sel, quit = run_welcome_and_list()
    if quit then
        return
    end

    if sel == 1 then
        if has_spell then
            local d = me:dialog(npc, "자네는 이미 금강퇴를 수련중이군.. 금강퇴는 다른 마법과 다르게 꾸준한 수련이 필요한 무술일세.", { prev = false, next = false })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            return
        end
        local sel2, btn2 = me:list(npc, "어쩌시겠소? 시주를 하시고 금강퇴를 전수받으시겠소?", {
            "물론입니다. 어서 전수하여 주십시오.",
            "아닙니다. 다음에 다시 찾아오겠습니다.",
        }, { prev = false })
        if btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
            return
        end
        if sel2 == 2 then
            me:dialog(npc, "그대 뜻이 그렇다면 어쩔수 없지. 앞길에 부처님의 뜻이 따르길...", { prev = false, next = false })
            return
        end
        local d = me:dialog(npc, "좋소. 그럼 시주 오만전을 받고 전수해주겟소. 자...", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        if me:money() < 50000 then
            me:dialog(npc, "이런, 금전이 모자라는구려. 돈을 모아 다시 찾아오시오.", { prev = false, next = false })
            return
        end
        me:money(me:money() - 50000)
        me:mkspell("금강퇴")
        d = me:dialog(npc, "무공의 진의는 수련을 통해서만 깨달을 수 있는법. 수련 방법을 자세히 알고 싶다면 내가 가르쳐주겠소.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "금강퇴는 강한 완력을 바탕으로 한 타격 무공이오. 반드시 근접하여 사용해야 한다오. 지금 금강퇴를 익혔다 하더라도 위력이 약할것이오.", { prev = false, next = false })
        return
    end

    if sel == 2 then
        if not has_spell then
            me:dialog(npc, "자네는 금강퇴를 아직 배우지 않았구려. 일단 전수부터 받아야 수련을 할 수 있다오.", { prev = false, next = false })
            return
        end
        me:dialog(npc, "일단 자네가 어느정도 위력의 금강퇴를 가지고 있는지. 능력이 어느정도인지 확인해봐야겠구려. 능력에 따라 그에 걸맞는 수련을 하도록 해주겠소.", { prev = false, next = true })
        local d = me:dialog(npc, "금강퇴의 수련도에 따라 내 적합한 조언을 해주도록 하겠네.", { prev = false, next = false })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if sel == 3 then
        local info = {
            "우선 금강퇴라는 무공은 바로 앞의 적에게 자신의 체력을 깍아 강력한 타격을 입히는 기술이오.",
            "어떤 무공이든 수련을 위해서는 계속 반복하여 시전하는 길밖에는 없소. 수련을 원하면 내가 각단계에 맞는 수련의서를 줄 것이오.",
            "수련의서를 받게 되면 일단 가지고 있던 금강퇴가 사라지게 되니 금강퇴의 시전은 수련서를 사용해야 가능하다오.",
            "이 수련의서를 이용하여 계속 금강퇴를 시전하다보면 그 진의를 깨달아가게 되고 이에 따라 수련도가 올라간다오.",
            "수련도가 100이 되었을 때 보다 강한 금강퇴를 몸에 익히게 되고 수련의서는 사라지게 될 것이오.",
            "모든 수련은 끈기와 노력이 없이는 불가능한법! 끊임없는 정진으로 꼭 금강퇴의 극의를 깨우치기 바라오.",
            "부처님이 언제나 함께 하시길. 나무아미타불.",
        }
        local i = 1
        while i >= 1 and i <= #info do
            local prev = (i > 1)
            local next = (i < #info)
            local button = me:dialog(npc, info[i], { prev = prev, next = next })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV and i > 1 then
                i = i - 1
            else
                i = i + 1
            end
        end
        return
    end
end