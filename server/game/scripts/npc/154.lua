-- npc: 혜천대사
function NPC_154(me, npc)
    if me:level() < 20 then
        me:dialog(npc, "자네는 소림사의 비기를 전수받기에 너무 약한것 같군...\n\n레벨이 20이 넘거든 다시 오게나.", { prev = false, next = false })
        return
    end

    local sel, btn = me:list(npc, "안녕하세요. 어떻게 오셨나요?", {
        "소림사백열장전수",
        "소림사선풍각전수",
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 1 then
        if me:spell("백열장") then
            local d = me:dialog(npc, "당신은 백열장을 배워 수련중이군요..", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            d = me:dialog(npc, "소림사의 무공은 수련을 할수록 그 위력이 엄청나진다오.. 부디 수련에 정진하여 더욱 강해지길 바라오...", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "소림사 본전의 뒤쪽에 수련장이 있소. 그곳에 가면 소림사 승려들과 수련할 수 있을 것이오..", { prev = false, next = false })
            return
        end
        local d = me:dialog(npc, "자네는 백열장을 모르나? 백열장은 소림사에서 가장 쉽게 배울수 있는 무공이라네. 백열장을 모른다면 소림사를 안다고 할 수 없지.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        d = me:dialog(npc, "백열장은 수련에 따라 그 위력이 천차만별이지. 소림사 뒤쪽의 수련장에서 수련할 수 있다네.", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        local sel2, btn2 = me:list(npc, "소림사에 10000전을 시주한다면 내 직접 백열장을 전수해주겠소. 어떻소?", {
            "좋습니다. 어서 가르쳐주십시오!",
            "괜찮습니다. 그럼 이만.",
        }, { prev = false })
        if btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
            return
        end
        if sel2 == 1 then
            if me:money() < 10000 then
                me:dialog(npc, "안타깝지만 자네 수중에 충분한 돈이 없구만. 돈을 가지고 다시 찾아오시오.", { prev = false, next = false })
                return
            end
            if me:spell("백열장") then
                me:dialog(npc, "이미 나에게 배운 것 같군.. 하지만 시주한 금액은 소림사에서 좋은 일에 쓰도록 하겠네.", { prev = false, next = false })
            else
                me:money(me:money() - 10000)
                me:mkspell("백열장")
                local d = me:dialog(npc, "여기있소. 피땀흘려 수련을 한다면 언젠가는 꼭 보상을 받을것이오.", { prev = false, next = true })
                if d == DIALOG_RESULT.QUIT then
                    return
                end
                me:dialog(npc, "수련장은 소림사 뒤쪽으로 가보면 승려들이 수련하고 있을것이오.", { prev = false, next = false })
            end
        else
            me:dialog(npc, "알겠소. 언젠가 꼭 필요하다면 그때 다시 찾아오시오.", { prev = false, next = false })
        end
        return
    end

    if sel == 2 then
        if me:spell("선풍각") then
            me:dialog(npc, "선풍각을 배워 수련중이구려. 선풍각은 수련에 따라 얼마든지 강해진다오. 수련을 게을리 하지 마시길. 나무아미타불..", { prev = false, next = false })
            return
        end
        local dialogs = {
            "자네는 백열장을 알고 있나? 백열장은 손으로 이용하는 것으로, 소림사에서 가장 쉽게 배울 수 있는 무공이지. 그리고 발을 이용해 공격을 하는 무공으로는 '선풍각'이라는 무공이 있다네.",
            "백열장은 멀리 있는 적을 공격할 수 있는 무공인 것에 비해, 선풍각은 회전하면서 주변에 있는 적들 모두를 공격하는 무공이지.",
            "선풍각의 수련은 백열장과는 다르지. 백열장은 수련을 하면 일성, 이성.. 올라가지만, 선풍각을 수련하면 '첨의 무공을 배운다네.",
            "즉, 선풍각을 수련하면 자네는 일성선풍각이지만 수련을 할때를 제외하고는 일성선풍각'첨을 사용할 수 있게 되는 것이지. 그리고, 일성선풍각을 수련하면 이성선풍각과 사냥에서는 이성선풍각'첨을 사용할 수 있게 되네.",
            "그러나 자신의 직업에 맞는 선풍각'첨의 최종 단계까지 올라가면 선풍각은 사라진다는걸 명심하게.",
            "그리고 선풍각'첨의 효과를 받기 위해서 선풍각을 두번 수련할 필요가 없지. 무슨 뜻인지 알겠는가?",
        }
        for i, msg in ipairs(dialogs) do
            local d = me:dialog(npc, msg, { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
        end
        local sel2, btn2 = me:list(npc, "소림사에 20000전을 시주한다면 내 직접 선풍각을 전수해주겠소. 어떻소?", {
            "좋습니다. 어서 가르쳐주십시오!",
            "괜찮습니다. 그럼 이만.",
        }, { prev = false })
        if btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
            return
        end
        if sel2 == 1 then
            if me:money() < 20000 then
                me:dialog(npc, "안타깝지만 자네 수중에 충분한 돈이 없구만. 돈을 가지고 다시 찾아오시오.", { prev = false, next = false })
                return
            end
            if me:spell("선풍각") then
                me:dialog(npc, "이미 나에게 배운 것 같군.. 하지만 시주한 금액은 소림사에서 좋은 일에 쓰도록 하겠네.", { prev = false, next = false })
            else
                me:money(me:money() - 20000)
                me:mkspell("선풍각")
                local d = me:dialog(npc, "여기있소. 피땀흘려 수련을 한다면 언젠가는 꼭 보상을 받을것이오.", { prev = false, next = true })
                if d == DIALOG_RESULT.QUIT then
                    return
                end
                me:dialog(npc, "수련장은 소림사 뒤쪽으로 가보면 승려들이 수련하고 있을것이오.", { prev = false, next = false })
            end
        else
            me:dialog(npc, "알겠소. 언젠가 꼭 필요하다면 그때 다시 찾아오시오.", { prev = false, next = false })
        end
    end
end