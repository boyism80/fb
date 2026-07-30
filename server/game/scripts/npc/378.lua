-- npc: 비밀
local quest = require('lib.quest')
local enum = require('lib.enum')
local ASSEMBLE_ITEMS = { "요술구슬", "상하막대", "좌우막대", "누름막대" }
local ASSEMBLE_FEE = 100000
local SELL_PRICE = 150000

return {
    on_click = function(me, npc)
        local q = me:quest(quest.QUEST_MAGIC_BALL)
        local param = (q and q:param()) or "0"

        if param == "0" then
            if me:level() < 35 then
                local btn = me:dialog(npc, "아이구, 이런... 모처럼 새로운 시장을 제일 먼저 개척하나 싶었더니만 괴물들이 나타나 버리다니.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "이를 어쩌면 좋지? 이건 죽여도 죽여도 끝이 없이 쏟아져나오니 장수를 할 수가 있나.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                me:dialog(npc, "아휴, 참... 신용만점, 정직만점을 자랑하는 내 인생이 큰오점을 남기게 생겼네...아이구...\n\n(어쩐지 못미덥고 거짓말쟁이같은 상인의 푸념은 끝이 없다.)", { prev = false, next = false })
                return
            end
            local btn = me:dialog(npc, "음? 요술놀이 구슬 특가 판매전? 아, 물론 하려고 했지. 그래, 하려고 했어\n그런데 건물이 다 세워지고 짐을 옮겨놓자마자 괴물들이 들이닥쳤지 뭔가? 북새통에 다 흩어지고 그나마 건진건 목숨밖에 없어.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "그래, 마침 좋은 기회군. 자네, 안에 들어가서 괴물들 좀 해치워주지 않겠나?\n대신 괴물들이 갖고 있는 요술놀이 구슬 부속품들을 가져오면, 내가 저렴하게 조립해주지", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "요술구슬, 좌우막대, 상하막대, 누름막대...이렇게 찾아오면 된다네. 되도록 괴물들을 많이 해치워주면 좋겠군.\n\n자, 그럼 부탁하네.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if q == nil then
                q = me:start_quest(quest.QUEST_MAGIC_BALL)
                if q == nil then
                    me:dialog(npc, "퀘스트를 시작할 수 없습니다.", { prev = false, next = false })
                    return
                end
            end
            if q then
                q:param("1")
            end
            return
        end

        if param == "1" then
            local sel, list_btn = me:list(npc, "오, 그래, 안쪽 상황은 좀 어떤가? 많이 해치웠나? 부속품은 구했고?", {
                "여긴 포기하는게 나을 것 같은데요.",
                "열심히 해치우는 중이에요.",
                "여기 부속품 다 구해왔습니다.",
            }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sel == 1 then
                me:dialog(npc, "뭐라구! 지금 제정신으로 하는 소리인가! 내가 여기 상거래 허가를 얻기위해 도대체 얼마나 많은 뇌물을 뿌렸는지 모르는군!", { prev = false, next = true })
                me:dialog(npc, "음? 아니아니, 방금 그 말은 잊어주게. 어흠어흠흠... 나는 한 사람의 자랑스러운 상인으로서 좋은 물건은 세상 모든 사람들이 사서 쓸 수 있도록 공급해야한다는 나의 철학을 지켜야만 하네. 따라서 여기에서 포기할 순 없지!", { prev = false, next = true })
                me:dialog(npc, "(어쩐지 못미덥고 거짓말쟁이같은 상인은 황급하게 말을 바꾸고 끝맺었다.)", { prev = false, next = false })
                return
            end
            if sel == 2 then
                me:dialog(npc, "그럼 수고해주게나.", { prev = false, next = false })
                return
            end
            if sel == 3 then
                local sel2, list_btn2 = me:list(npc, "자, 10만전에 조립을 부탁하겠나? 응? 자 어쩔 생각이지? 응? 어서 말해보시게.", {
                    "아 그래요? 그럼 괴물퇴치 비용을 지불해주세요.",
                    "10만전이라... 자, 그럼 10만전에 조립해주세요.",
                }, { prev = false })
                if list_btn2 == DIALOG_RESULT.QUIT then
                    return
                end
                if sel2 == 1 then
                    me:dialog(npc, "뭐? 뭐라구! 괴물 퇴치 비용을 달라구! 아니, 이런 치사스런 사람을 봤나! 너무한거 아닌가! 남의 불행을 이용하다니!\n\n이런 천인공노할 사람을 봤나! 세상에나! 세상에나! 아이고, 억울해! 억울해서 배가 다 아프고, 머리까지 아프네!", { prev = false, next = true })
                    me:dialog(npc, "(어쩐지 못미덥고 거짓말쟁이에 치사스럽고 천인공노할 상인의 분노를 무시해버렸다.)", { prev = false, next = false })
                    return
                end
                if sel2 == 2 then
                    local cost_item = {}
                    for _, name in ipairs(ASSEMBLE_ITEMS) do
                        cost_item[name] = 1
                    end
                    local code = me:exchange(
                        { ['item'] = cost_item, ['money'] = ASSEMBLE_FEE },
                        { ['item'] = { ["요술놀이구슬"] = 1 } }
                    )
                    if code == enum.exchange_result.LACK_COST then
                        me:dialog(npc, "재료를 다 구해오지 않았잖아! 요술구슬과 상하막대, 좌우막대와 누름막대라네! 그리고 10만전도 말이야!", { prev = false, next = false })
                        return
                    end
                    if code == enum.exchange_result.LACK_CAPACITY then
                        me:dialog(npc, "소지품이 가득 차서 요술놀이구슬을 받을 수 없어요. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
                        return
                    end
                    if q then
                        q:param("2")
                    end
                    me:dialog(npc, "자 여기 있네. 사실 말이야 바른 말이지. 내가 자금 사정이 급하지 않았더라면...\n\n(어쩐지 못미덥고 거짓말쟁이같은데다가 생색까지 내는 상인을 무시하고 돌아섰다.)", { prev = false, next = false })
                end
            end
            return
        end

        local sel, list_btn = me:list(npc, "음? 무슨 일이지?", {
            "이 요술놀이 구슬은 얼마나 하나요?",
            "부속을 또 모아왔는데, 조립해줘요.",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == 1 then
            me:dialog(npc, "오, 그래? 필요없는가 본데, 내가 사줄 수도 있지. 이야, 이미 사람 손탄거라 비싸고 주고 싶어도 비싸게는 못줘.", { prev = false, next = true })
            me:dialog(npc, "아니, 뭐. 아직 보급형이 나오지 않아서 등록비도 만만찮고 말이야. 사실 말이야 바른 말이지. 이런 물건은 높으신 분이나 그 자제분들이나 가지고 노는 물품이라구. 어떤가? 지금이라면 내가 인심 팍팍 써서 30만전에 사주지.", { prev = false, next = true })
            local sel2, list_btn2 = me:list(npc, "자 이런 기회는 다시 없다구. 이걸 어디가서 팔겠냐구. 응? 자자, 나한테 팔아", {
                "15만전이라... 좋아요, 팔께요.",
                "원가가 600만전이라고 들었는데요?",
            }, { prev = false })
            if list_btn2 == DIALOG_RESULT.QUIT then
                return
            end
            if sel2 == 1 then
                local code = me:exchange(
                    { ['item'] = { ["요술놀이구슬"] = 1 } },
                    { ['money'] = SELL_PRICE }
                )
                if code == enum.exchange_result.LACK_COST then
                    me:dialog(npc, "요술놀이구슬이 없지 않은가? 장난하나? 지금?", { prev = false, next = false })
                    return
                end
                if code == enum.exchange_result.LACK_CAPACITY then
                    me:dialog(npc, "금전을 받을 여유가 없군요.", { prev = false, next = false })
                    return
                end
                me:dialog(npc, "아주 현명한 선택이야. 자, 15만전 여기있네. 자네 정말 득본거야. 흐흐흐흐흐흐흐.", { prev = false, next = false })
            else
                me:dialog(npc, "자네! 대체 어디서 그런 소리를 들은겐가! 이것은 내가 인심을 팍팍 써줘서 15만전이지 딴데 가서는 어디서 팔지도 못해! 음음.. 내가 특별히 15만전에 사주는데도 어디서 헛소리를 들어 왔는진 모르겠군..", { prev = false, next = false })
            end
            return
        end
        if sel == 2 then
            local cost_item = {}
            for _, name in ipairs(ASSEMBLE_ITEMS) do
                cost_item[name] = 1
            end
            local code = me:exchange(
                { ['item'] = cost_item, ['money'] = ASSEMBLE_FEE },
                { ['item'] = { ["요술놀이구슬"] = 1 } }
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, "재료를 다 구해오지 않았잖아! 요술구슬과 상하막대, 좌우막대와 누름막대라네! 그리고 10만전도 말이야!", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 요술놀이구슬을 받을 수 없어요. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
                return
            end
            me:dialog(npc, "자 여기 있네. 사실 말이야 바른 말이지. 내가 자금 사정이 급하지 않았더라면...\n\n(어쩐지 못미덥고 거짓말쟁이같은데다가 생색까지 내는 상인을 무시하고 돌아섰다.)", { prev = false, next = false })
        end
    end
}
