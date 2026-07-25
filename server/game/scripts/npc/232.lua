-- npc: 상해주민
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local q = me:quest(quest.QUEST_HWANGBIYEON)

        if q == nil then
            me:dialog(npc, '안녕하신가요?', { prev = false, next = true })
            return
        end

        if q:step() == 0 then
            me:dialog(npc, '안녕하신가요?', { prev = false, next = true })
            return
        end

        if q:step() == 1 then
            ::NPC_232_0000::
            local sel, lb = me:list(npc, '안녕하십니까. 허허, 아까부터 뭔가 찾으시는 눈치신데... 뭘 그리 찾으시는지요?', { '황비연이라는 사람을 찾고 있습니다.', '아무 것도 아닙니다.' }, { prev = false })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            ::NPC_232_0001::
            local btn = me:dialog(npc, '황비연? ...음, 황비연이라면 요즘 탐관오리의 재물을 훔쳐 가난한 사람들에게 나눠 준다는 의적 아닙니까?', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0000
            end
            ::NPC_232_0002::
            sel, lb = me:list(npc, '참으로 훌륭한 사람이지요. 그런데 그는 왜 찾으십니까?', { '그를 잡아서 현상금을 타려고 합니다.', '꼭 한 번 만나 보고 싶은 사람입니다.' }, { prev = true })
            if lb == DIALOG_RESULT.QUIT then
                return
            end
            if lb == DIALOG_RESULT.PREV then
                goto NPC_232_0001
            end
            if sel == 1 then
                me:dialog(npc, '황비연이 현상금이 높긴 하죠... 하지만 쉽게 잡기는 어려울 겁니다.', { prev = false, next = true })
                return
            end
            if sel ~= 2 then
                return
            end
            ::NPC_232_0003::
            btn = me:dialog(npc, '그렇습니까! 하하. 아마 황비연에게 도움을 받으셨었던 모양이지요?\n\n실은 저도 그를 꼭 한 번 만나 보고 싶지만, 한 번도 본 적이 없답니다. 하하...', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0002
            end
            ::NPC_232_0010::
            btn = me:dialog(npc, '......', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0003
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_232_0011::
            btn = me:dialog(npc, '......그를 꼭 만나고 싶으시다면, 제 부탁 하나만 들어주시겠습니까? 아시다시피 황비연은 신출귀몰하고, 사람들이 모두 그를 감싸주기 때문에 아무도 그의 행방을 모르지요.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0010
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_232_0012::
            btn = me:dialog(npc, '하지만 저는 사실 황비연을 좀 알고 있습니다. 당신이 제 부탁을 들어주신다면 황비연과 만나 보실 수 있도록 자리를 만들어 보겠습니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0011
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            sel, lb = me:list(npc, '...자, 어떻습니까?', { '알겠습니다, 그렇게 하죠.', '싫습니다. 어째 꺼림칙하군요.' }, { prev = false })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            ::NPC_232_0020::
            btn = me:dialog(npc, '감사합니다. 그럼...\n\n이 노비문서를 도삭산 100층 주막에 계시는 분께 전해 주십시오. 일이 끝난 다음 제게 오시면 약속대로 정보를 드리겠습니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0012
            end
            ::NPC_232_0021::
            btn = me:dialog(npc, '그럼 수고스러우시겠지만 잘 부탁드립니다.', { prev = true, next = false })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0020
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if me:mkitem('노비문서', 1) == nil then
                me:dialog(npc, '소지품이 가득 차서 노비문서를 줄 수 없습니다.', { prev = false, next = true })
                return
            end
            q:step(2)
            me:push_achievement(39, '도삭산 100층으로가 노비문서를 전해주자.', 7, 16)
            return
        end

        if q:step() == 2 then
            me:dialog(npc, '아직 도삭산 100층에 계신분을 만나지 못하셨나 보군요.', { prev = false, next = true })
            return
        end

        if q:step() == 3 then
            if not me:has_items('보패', 1) then
                me:dialog(npc, '아직 도삭산 100층에 계신분을 만나지 못하셨나 보군요.', { prev = false, next = true })
                return
            end
            ::NPC_232_0023::
            local sel, lb = me:list(npc, '문서는 전해 주고 오셨는지요?', { '예, 확실히 전해 줬습니다.', '아니오, 아직...' }, { prev = false })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            ::NPC_232_0024::
            local btn = me:dialog(npc, '그렇군요. 감사합니다. 수고하셨습니다... 그런데 이 보패는 웬 것입니까?', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0023
            end
            ::NPC_232_0025::
            sel, lb = me:list(npc, '예? 도재영님이 이것을 제게 전해달라고 하셨다고요? 정말입니까?', { '예, 당신을 자랑스러워하셨습니다. 황비연씨.' }, { prev = true })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            if lb == DIALOG_RESULT.PREV then
                goto NPC_232_0024
            end
            ::NPC_232_0026::
            btn = me:dialog(npc, '...다 알고 계셨군요. 하하. 이거 들켜버렸네요. 죄송합니다. 예, 제가 황비연입니다.\n\n부탁도 들어 주셨고, 제 할 일도 다 끝났습니다. 많은 사람을 도왔고, 나으리께서도 저를 자랑스럽게 여겨 주신다니 여한이 없습니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0025
            end
            ::NPC_232_0027::
            sel, lb = me:list(npc, '자, 이제 저를 체포하셔서 관아에 넘겨 주십시오.', { '그럴 수는 없습니다.' }, { prev = true })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            if lb == DIALOG_RESULT.PREV then
                goto NPC_232_0026
            end
            ::NPC_232_0028::
            sel, lb = me:list(npc, '예? 무슨 말씀이신지...', { '당신은 의적으로서 많은 사람을 도왔죠.' }, { prev = true })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            if lb == DIALOG_RESULT.PREV then
                goto NPC_232_0027
            end
            ::NPC_232_0029::
            sel, lb = me:list(npc, '......', { '그런 사람을 체포할 수는 없습니다.' }, { prev = true })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            if lb == DIALOG_RESULT.PREV then
                goto NPC_232_0028
            end
            ::NPC_232_0030::
            sel, lb = me:list(npc, '음......하지만 의적이라고는 해도 도둑질은 나쁜 일입니다. 저는 죗값을 치러야 합니다.', { '앞으로 좋은 일을 더 많이 하시면 되지 않겠습니까?' }, { prev = true })
            if lb == DIALOG_RESULT.QUIT or sel ~= 1 then
                return
            end
            if lb == DIALOG_RESULT.PREV then
                goto NPC_232_0029
            end
            ::NPC_232_0031::
            btn = me:dialog(npc, '아... 정말로 그럴까요? 부끄럽습니다...\n\n하지만 당신의 이야기를 들어 보니 그런 것 같기도 합니다. 그게 더 보람있는 길이란건 확실하겠지요...', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0030
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_232_0032::
            btn = me:dialog(npc, '그렇다면...\n\n이것을 가져가 주십시오. 제가 쓰던 머리띠입니다. 이것을 관아에 가져가시면 제가 죽었다고 생각할 것이고, 아마 제 현상금을 받으실 수 있을 겁니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0031
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_232_0033::
            btn = me:dialog(npc, '보답이라고 하기엔 뭣하지만... 훔친 재물은 전부 가난한 사람들에게 나눠줬기 때문에 드릴 것이 없군요. 하하...', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0032
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, '저는 더 많은 사람들을 돕도록 하겠습니다. 물론 이제 도둑질은 하지 않겠습니다. 떳떳한 방법으로 사람들을 도울 겁니다. 정말 감사합니다.', { prev = true, next = false })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_232_0033
            end
            local code = me:exchange(
                { ['item'] = { ['보패'] = 1 } },
                { ['item'] = { ['황비연의머리띠'] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '보패를 가지고 있지 않으시군요.', { prev = false, next = true })
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 황비연의머리띠를 줄 수 없습니다.', { prev = false, next = true })
                return
            end
            q:step(4)
            me:push_achievement(39, '황비연의머리띠를 관아에 가져가자.', 7, 16)
            return
        end

        me:dialog(npc, '안녕하신가요?', { prev = false, next = true })
    end
}
