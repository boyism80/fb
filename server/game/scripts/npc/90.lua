-- npc: 천성현
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local OPT_PROMOTE = '3차 승급을 원합니다'
        local sel, btn = me:pursuit(npc, '안녕하세요. 어떻게 오셨나요?', { OPT_PROMOTE })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= OPT_PROMOTE then
            return
        end

        if me:promotion() < 1 then
            me:dialog(npc, '2차 승급을 한 뒤에 다시 오세요.', { prev = false, next = true })
            return
        end

        local q = me:quest(quest.QUEST_PROMOTION_3RD)
        if q == nil then
            sel = me:list(npc, '당신은 2차승급을 하신 분이시군요. ...제 이야기를 좀 들어보시겠어요?', {
                '네, 이야기 하세요.',
                '아니오. 음 바빠서.',
            })
            if sel == nil then
                return
            end
            if sel ~= 1 then
                return
            end

            if me:dialog(npc, '길림성이란 곳이 있습니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '이곳은 청룡 백호, 주작 현무의 신전이 각 방위마다 붙어있으며 조화로운 곳으로 알려져있습니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '그런데 어쩐일인지 이들 신수들의 힘이 예전보다 약해지고 있는데 이유를 알아내지 못하고 있습니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '그리고 길림성에 살고 있던 동물들도 이상하게 변했다고 하더군요.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end

            sel = me:list(npc, '당신이 이것을 조사해 주시겠습니까?', { '네, 제가 조사하겠습니다.' })
            if sel == nil or sel ~= 1 then
                return
            end

            if me:dialog(npc, '그럼 제가 당신에게 부탁했다는 증거로 증표를 드리겠습니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end

            local q = me:quest(quest.QUEST_PROMOTION_3RD)
            if me:has_items('승급의서장', 1) or (q and q:step() >= 1) then
                me:dialog(npc, '이미 증표를 드린 적이 있습니다. 잃어버리지 않도록 하십시오.', { prev = false, next = true })
                return
            end
            if me:mkitem('승급의서장', 1) == nil then
                me:dialog(npc, '소지품이 가득 차서 증표를 받을 수 없습니다.', { prev = false, next = true })
                return
            end
            q = me:start_quest(quest.QUEST_PROMOTION_3RD)
            if q then
                q:step(1)
            end

            if me:dialog(npc, '제가 드린 이 증표는 절대 잃어버리셔선 안됩니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '그럼 길림성으로 가보십시오. 거기서 당신의 신수의 제단을 찾아가 계시를 받아야 할 것입니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '각 신수의 제단은 동쪽에 청룡의 제단, 서쪽에 백호의 제단, 남쪽에 주작의 제단, 북쪽에 현무의 제단이 있습니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '그럼 성공하시길 잘 다녀오시길 바랍니다..', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        if q:completed() then
            return
        end


        if q:step() == 1 then
            local materials = {['고대마법서\'상'] = 1, ['고대마법서\'하'] = 1}
            if not me:has_items(materials) then
                me:dialog(npc, '아직 고대마법서를 찾지 못하셨군요.', { prev = false, next = true })
                return
            end

            if not me:rmitem(materials, ITEM_DELETE_TYPE.GIVE) then
                me:dialog(npc, '고대마법서를 찾지 못하셨군요.', { prev = false, next = true })
                return
            end
            q:step(2)

            local dialogs = {
                '아.. 길림성의 신수의 부탁을 들어주고 오셨나보군요..',
                '수고하셨습니다. 하지만 아직 이 일이 끝난 것은 아닙니다.',
                '당신이 지금까지 싸워오신 악의 무리들은 거대한 암흑의 존재에서 부터 비롯된것,',
                '그 암흑의 존재를 처치하지 못한다면 악의 무리들은 다시금 부활할 것입니다.',
                '그럼 마법책을 읽어드리겠습니다.',
                "'동쪽과 북쪽에서 나타난 어둠의 세력은 서쪽으로부터 오는 힘에 근원을 두고 있다.",
                '서쪽의 어둠은 깊은 미궁속에 숨어있는 존재로 부터 오는 것이다.',
                '길림성의 서쪽에 있는 미궁을 말하는 것 같군요. 아마 이곳에 암흑왕이 숨어 있는 것 같군요.',
                '그럼 이곳으로 가시기 전에 제가 당신을 축복해드리겠습니다.',
                '임무를 모두 마치시고 바로 저에게로 돌아오십시오.',
                '암흑의 존재는 그의 졸개들과 마찬가지로 시간이 지나면 부활해 버립니다.',
                '그럼 행운을 빕니다.',
            }
            for _, msg in ipairs(dialogs) do
                if me:dialog(npc, msg, { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                    return
                end
            end
        end

        if q:step() == 2 then
            local materials = {['암흑왕의봉인'] = 1}
            if not me:has_items(materials) then
                me:dialog(npc, '아직 암흑왕의봉인을 찾지 못하셨군요.', { prev = false, next = true })
                return
            end

            if me:dialog(npc, '아.. 성공하셨군요!! 당신이 암흑의 존재를 처치한 것을 옥황상제님께서도 기뻐하실 것입니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end

            local code = me:exchange(
                { ['item'] = materials },
                { ['item'] = { ['천성현의증표'] = 1 } }
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, '암흑왕의봉인을 찾지 못하셨군요.', { prev = false, next = true })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 증표를 받을 수 없습니다.', { prev = false, next = true })
                return
            end
            q:complete()

            if me:dialog(npc, '그럼 제가 드리는 증표를 가지고 천상계로 가시면 당신을 다음 단계로 승급하도록 해드리겠습니다.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '천상제일계의 선녀들에게로 가 보십시오.', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
        end
    end
}
