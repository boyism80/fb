-- npc: 산소소탕대장
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        local ACHIEVEMENT_OXYGEN = 4
        local MIN_LEVEL = 50

        local tiers = {
            { item = '연청산소의뿔', count = 50, money = 3000, next_name = '백산소', next_count = 50, next_legend = '백산소의 뿔을 50개 모아가자.' },
            { item = '백산소의뿔', count = 50, money = 5000, next_name = '황산소', next_count = 30, next_legend = '황산소의 뿔을 30개 모아가자.' },
            { item = '황산소의뿔', count = 30, money = 10000, next_name = '청산소괴', next_count = 30, next_legend = '청산소괴의 뿔을 30개 모아가자.' },
            { item = '청산소괴의뿔', count = 30, money = 15000, next_name = '적산소괴', next_count = 30, next_legend = '적산소괴의 뿔을 30개 모아가자.' },
            { item = '적산소괴의뿔', count = 30, money = 20000, next_name = '녹산소괴', next_count = 30, next_legend = '녹산소괴의 뿔을 30개 모아가자.' },
        }

        local q = me:quest(quest.QUEST_OXYGEN)
        local btn, sel

        if q == nil then
            ::NPC_211_0001::
            btn = me:dialog(npc, '외부에서 오신분이십니까? 남경 지역을 지나실때 조심하시기 바랍니다.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_211_0010::
            btn = me:dialog(npc, '현재 산소라는 요괴 때문에 백성들이 피해가 이만저만이 아니라하여 소탕작전을 위해 출동하였습니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_211_0001
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_211_0011::
            btn = me:dialog(npc, '하지만 워낙 산소의 종류와 수가 많고 그에 비해 병사의 수가 터무니없이 적어 어려움이 많습니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_211_0010
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_211_0012::
            sel, btn = me:list(npc, '산소를 소탕하는데 힘을 빌려주시지 않겠습니까? 도와주시면 보상금을 드리도록 하겠습니다.', { '좋습니다! 제 힘이 도움이 된다면 기꺼이..', '죄송하지만 갈길이 멀답니다. 이만.' }, { prev = false })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_211_0011
            end
            if sel == nil or sel ~= 1 then
                if sel == 2 then
                    me:dialog(npc, '그러시군요.. 가시는 길 조심히 살펴서 가세요.', { prev = false, next = true })
                end
                return
            end
            btn = me:dialog(npc, '우선 산소들을 상대하실 수 있는지 봐야겠습니다.', { prev = false, next = true })
            if me:level() < MIN_LEVEL then
                me:dialog(npc, '아직 산소들을 상대하시기엔 무리인 것 같습니다. 마음만은 고맙게 받겠습니다.', { prev = false, next = true })
                return
            end
            ::NPC_211_0020::
            btn = me:dialog(npc, '좋습니다! 이정도라면 충분합니다. 산소들은 종류마다 뿔을 가지고 있습니다. 이 뿔은 증표로 모아오시면 그에 따른 보상금을 드리겠습니다.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_211_0021::
            btn = me:dialog(npc, '우선 약한 산소부터 시작하도록 하죠. 연청산소가 가장 약합니다. 연청산소의 뿔을 50개 모아와 주시기 바랍니다.', { prev = false, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_211_0020
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            q = me:start_quest(quest.QUEST_OXYGEN)
            if q == nil then
                me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
                return
            end
            q:step(1)
            me:push_achievement(ACHIEVEMENT_OXYGEN, '연청산소의 뿔을 50개 모아가자.', 7, 1)
            return
        end

        if q:completed() then
            me:dialog(npc, '외부에서 오신분이십니까? 남경 지역을 지나실때 조심하시기 바랍니다.', { prev = false, next = true })
            return
        end

        local step = q:step()

        if step >= 1 and step <= 5 then
            local t = tiers[step]
            local code = me:exchange(
                { ['item'] = { [t.item] = t.count } },
                { ['money'] = t.money }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, string.format('아직 %s %d개를 모아오지 못하신 것 같군요?', t.item, t.count), { prev = false, next = true })
                return
            elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 보상금을 받을 수 없습니다.', { prev = false, next = true })
                return
            end
            q:step(step + 1)
            me:push_achievement(ACHIEVEMENT_OXYGEN, t.next_legend, 7, 1)
            btn = me:dialog(npc, string.format('정말 수고하셨습니다. 보상금 여기있습니다. 다음은 %s입니다. %s의뿔 %d개를 모아와 주십시오.', t.next_name, t.next_name, t.next_count), { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, '갈수록 보상금이 높아지니 부탁드립니다.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        if step == 6 then
            if not me:has_items('녹산소괴의뿔', 30) then
                me:dialog(npc, '아직 녹산소괴의뿔 30개를 모아오지 못하신 것 같군요?', { prev = false, next = true })
                return
            end
            ::NPC_211_0030::
            btn = me:dialog(npc, '정말 감사드립니다. 여기까지 도와주셔서 저희에겐 너무나 큰 도움이 되었답니다.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_211_0031::
            btn = me:dialog(npc, '상당히 강하신분 같은데 한가지 더 부탁드려도 될까요. 산소들의 우두머리인 산소괴왕은 저희가 건드릴 수가 없답니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_211_0030
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_211_0032::
            sel = me:list(npc, '산소괴왕을 처리해주시겠습니까?', { '알겠습니다. 제가 처리해보도록 하죠.', '죄송합니다. 그건 저에게 무리일듯하군요.' }, { prev = false })
            if sel == nil or sel ~= 1 then
                if sel == 2 then
                    me:dialog(npc, '그러시다면야 할 수 없죠.', { prev = false, next = true })
                end
                return
            end
            local code2 = me:exchange(
                { ['item'] = { ['녹산소괴의뿔'] = 30 } },
                { ['money'] = 30000 }
            )
            if code2 == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '아직 녹산소괴의뿔 30개를 모아오지 못하신 것 같군요?', { prev = false, next = true })
                return
            elseif code2 == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 보상금을 받을 수 없습니다.', { prev = false, next = true })
                return
            end
            q:step(7)
            me:push_achievement(ACHIEVEMENT_OXYGEN, '산소괴왕의 뿔을 구하자.', 7, 1)
            me:dialog(npc, '산소괴왕은 다른 산소들과는 차원이 다릅니다. 더구나 잘 나타나지 않아 찾기가 더 힘들답니다.\n\n산소괴왕의뿔을 가져오시면 귀한 물건을 드리도록 하죠. 부디 조심하시기 바랍니다.', { prev = false, next = true })
            return
        end

        if step == 7 then
            if not me:has_items('산소괴왕의뿔', 1) then
                me:dialog(npc, '아직 산소괴왕의뿔을 구하시지 못하셨군요. 부디 조심하시길 바랍니다.', { prev = false, next = true })
                return
            end
            btn = me:dialog(npc, '정말 대단하군요. 설마 정말 산소괴왕을 잡아오시리라고는. 이로써 큰 걱정을 덜게 되었군요.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local code3 = me:exchange(
                { ['item'] = { ['산소괴왕의뿔'] = 1 } },
                { ['item'] = { ['흑영패도'] = 1 } }
            )
            if code3 == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '산소괴왕의뿔을 가지고 있지 않으시군요.', { prev = false, next = true })
                return
            elseif code3 == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 흑영패도를 받을 수 없습니다.', { prev = false, next = true })
                return
            end
            q:complete()
            me:push_achievement(ACHIEVEMENT_OXYGEN, '산소소탕 작전을 성공적으로 수행하였다.', 7, 1)
            btn = me:dialog(npc, '정말 감사드립니다. 그 보답으로 이 보검을 드리죠. 황제에게 특별히 하사받은것인데 저에겐 과분한 물건인듯 하군요.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, '부디 좋은 곳에 써주시기 발바니다. 그럼 안녕히.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return
        end
    end
}
