-- npc: 피리부는소년
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_PAMASPIRI)
        if q == nil then
            me:dialog(npc, "삐리리- 삘릴리리- 삘릴리리리- 삐- 삐---\n내 피리 부는 솜씨가 어떠냐!", { prev = false, next = false })
            return
        end
        if q:completed() then
            me:dialog(npc, "사탕이 먹고 싶어.", { prev = false, next = false })
            return
        end

        local step = q:step()
        if step == 1 then
            ::NPC_489_0001::
            local sel, btn = me:list(npc, "삐리리- 삘릴리리- 삘릴리리리- 삐- 삐---\n내 피리 부는 솜씨가 어떠냐!", { "꼬마야, 그 피리 어디서 났니?", "되게 시끄럽네..." }, { prev = false })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if sel == nil or sel == 2 then
                return
            end

            ::NPC_489_0002::
            sel, btn = me:list(npc, "......응? 이 피리 말이야?\n이 피리는 내꺼야!", { "그래, 어디서 났는지 가르쳐줄래?", "그래, 너 가져라. 난 시끄러워서 가야겠다..." }, { prev = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_489_0001
            end
            if sel == nil or sel == 2 then
                return
            end

            ::NPC_489_0003::
            sel, btn = me:list(npc, "저~기 집 근처에서 주웠어. 주운 거니까 내꺼야. 안 줘.", { "꼬마야, 그 피리는 말이야...", "말이 안 통하는 꼬마로군..." }, { prev = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_489_0002
            end
            if sel == nil or sel == 2 then
                return
            end

            ::NPC_489_0004::
            local button = me:dialog(npc, "......어, 그렇게 중요한 피리였어? 그런 줄은 몰랐는걸. 하지만 내가 주운 거니까 내꺼야. 그냥은 안 줘!", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_489_0003
            end

            q:step(2)
            me:push_achievement(48, "파마의 피리를 찾자(사탕을 구하자).", 7, 1)

            ::NPC_489_0005::
            button = me:dialog(npc, "뭔가 좋은 물건을 가져오면 이 피리랑 바꿔 줄게. 뭐가 좋을까? 음... 그래. 사탕! 난 사탕이 먹고 싶어! 사탕을 가져다주면 피리를 줄게!", { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_489_0004
            end

            ::NPC_489_0006::
            me:dialog(npc, "사탕이 어디에 있냐구? 나도 몰라! 나도 누가 줘서 한 번밖에 먹어본 적이 없거든. 사람들한테 물어봐.\n\n삘릴리- 삐리리리리리리- 삐- 삐- 삐리리리-- 삐--\n어떠냐! 내 피리 부는 솜씨가!", { prev = false, next = false })
            return
        end

        if step == 2 then
            me:dialog(npc, "사탕을 가져오지 않으면 피리를 주지 않을테야!", { prev = false, next = false })
            return
        end

        if step == 3 then
            if not me:has_items("꿀사탕", 1) then
                me:dialog(npc, "사탕을 가져오지 않으면 피리를 주지 않을테야!", { prev = false, next = false })
                return
            end

            ::NPC_489_0007::
            local sel, btn = me:list(npc, "앗, 손에 들고 있는 그거 사탕이지? 내 피리랑 바꾸자!", { "그래, 사탕을 줄 테니 피리를 이리 주렴.", "안 돼." }, { prev = false })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if sel == nil or sel == 2 then
                me:dialog(npc, "난 사탕이 먹고 싶단 말이야!", { prev = false, next = false })
                return
            end

            local code = me:exchange(
                { ['item'] = { ["꿀사탕"] = 1 } },
                { ['item'] = { ["파마의피리"] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, "사탕을 가져오지 않으면 피리를 주지 않을테야!", { prev = false, next = false })
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 줄 수 없네요.", { prev = false, next = false })
                return
            end

            q:step(4)
            me:push_achievement(48, "파마의 피리를 찾자(파마의 피리를 돌려주자).", 7, 1)

            ::NPC_489_0008::
            button = me:dialog(npc, "좋았어! 나중에 딴말하기 없기!", { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_489_0007
            end

            me:dialog(npc, "사탕은 정말 맛있어! 히히......", { prev = false, next = false })
            return
        end

        me:dialog(npc, "사탕이 먹고 싶어.", { prev = false, next = false })
    end
}
