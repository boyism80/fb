-- npc: 요선도인
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "백두촌이 이상한 기운으로 뒤덮혀 정령들이 이성을 잃기 전 까지만 해도, 나는 그들의 신령한 기운을 모아 이런저런 무기와 장비를 만들곤 했었소.", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "하지만 최근들어 정령들이 암흑의 기운으로 폭주하고 난 뒤로... 사람들을 가차없이 공격하기에, 나는 그들이 기거하는 동굴의 앞에서 이렇게 정령들을 관찰하고 있소.", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "요일의 정령들의 기운이 담긴 돌을 백개씩 가져오면, 내가 할 수 있는 것을 해 주겠소...", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local STONES = { "일요지석", "월요지석", "화요지석", "수요지석", "목요지석", "금요지석", "토요지석" }
        local RECIPES_7 = {
            { main = "일요지석", product = "일광방패", desc = "일광방패는 해의 기운이 담긴 극양의 방패로서, 체력을 보강해주는 강력한 방패라고 말 할 수 있소.\n\n또한 일요의 기운이 담겨있으므로 [일요지석] 100개와, 이를 보강해줄 다른 지석 100개가 필요하오." },
            { main = "월요지석", product = "월광방패", desc = "월광방패는 달의 기운이 담긴 극음의 방패로서, 마력을 보강해주는 강력한 방패라고 말 할 수 있소.\n\n또한 월요의 기운이 담겨있으므로 [월요지석] 100개와, 이를 보강해줄 다른 지석 100개가 필요하오." },
            { main = "화요지석", product = "화분천검", desc = "화분천검은 불의 기운이 담긴 검으로써, 전사를 위한 무기라고 말 할 수 있소.\n\n또한 화요의 기운이 담겨있으므로 [화요지석] 100개와, 이를 보강해줄 다른 지석 100개가 필요하오." },
            { main = "수요지석", product = "수류곡도", desc = "수류곡도는 물의 기운이 담긴 도이므로, 도적을 위한 무기라고 말 할 수 있소.\n\n또한 수요의 기운이 담겨있으므로 [수요지석] 100개와, 이를 보강해줄 다른 지석 100개가 필요하오." },
            { main = "목요지석", product = "목근정곤", desc = "목근정곤은 나무의 기운이 담긴 곤봉으로써, 도사를 위한 무기라고 말 할 수 있소.\n\n또한 목요의 기운이 담겨있으므로 [목요지석] 100개와, 이를 보강해줄 다른 지석 100개가 필요하오." },
            { main = "금요지석", product = "금기련봉", desc = "금기련봉은 쇠의 기운이 담긴 봉으로써, 주술사를 위한 무기라고 말 할 수 있소.\n\n또한 금요의 기운이 담겨있으므로 [금요지석] 100개와, 이를 보강해줄 다른 지석 100개가 필요하오." },
            { main = "토요지석", product = "토선투구", desc = "토선투구는 흙의 기운이 담긴 투구으로써, 모든 직업을 위한 투구라고 말 할 수 있소.\n\n또한 토요의 기운이 담겨있으므로 [토요지석] 100개와, 이를 보강해줄 다른 지석 100개가 필요하오." },
        }

        local sel, btn = me:list(npc, "어떤 것을 만들고 싶은지 나에게 말 해 주시오.", {
            "일광방패", "월광방패", "화분천검", "수류곡도", "목근정곤", "금기련봉", "토선투구", "칠요구륜", "팔세지도", "팔요천의옷",
        }, { prev = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            return
        end

        if sel >= 0 and sel <= 7 then
            local r = RECIPES_7[sel]
            local d = me:dialog(npc, r.desc, { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            local sub_sel, sub_btn = me:list(npc, "[" .. r.product .. "]를 만들기 위해서는 [" .. r.main .. "] 100개와, 다른 지석 100개가 필요하오. 무엇을 주시겠소?", STONES, { prev = true })
            if sub_btn == DIALOG_RESULT.QUIT then
                return
            end
            local other = STONES[sub_sel]
            if other == r.main then
                me:dialog(npc, "같은 기운의 지석으로 만들 수 없소. 다른 것을 선택 해 주시오.", { prev = false, next = false })
                return
            end
            d = me:dialog(npc, "그렇군... [" .. r.main .. "]과 [" .. other .. "]으로 [" .. r.product .. "]를 만들어 보겠소. 다만 성공 확률은 삼할이 채 되지 않으니 실패하더라도 양해해 주시오...", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            local cost_7 = { [r.main] = 100, [other] = 100 }
            local reward = nil
            if math.random(1, 100) < 30 then
                reward = { ['item'] = { [r.product] = 1 } }
            end
            local code = me:exchange({ ['item'] = cost_7 }, reward)
            if code == enum.exchange_result.LACK_COST then
                if not me:has_items(r.main, 100) then
                    me:dialog(npc, r.main .. "이 부족합니다.", { prev = false, next = false })
                elseif not me:has_items(other, 100) then
                    me:dialog(npc, other .. "이 부족합니다.", { prev = false, next = false })
                else
                    me:dialog(npc, r.main .. "이나 " .. other .. "을 제거할 수 없소.", { prev = false, next = false })
                end
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 [" .. r.product .. "]를 줄 수 없소.", { prev = false, next = false })
                return
            end
            if reward == nil then
                me:dialog(npc, "이런... [" .. r.product .. "]를 만드는데 실패하고 말았소. 미안하게 됐소...", { prev = false, next = false })
            else
                me:dialog(npc, "[" .. r.product .. "]를 성공적으로 만들었소. 한번 살펴 보시게나....", { prev = false, next = false })
            end
            return
        end

        if sel == 8 then
            local d = me:dialog(npc, "칠요구륜은 모든 정령들의 기운을 혼합한 팔찌로써 모든 직업을 위한 팔찌라고 말할 수 있소..\n\n또한 모든의 기운이 불안정하게 담겨있으므로 모든 요일지석 100개가 필요하오.", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            local sub_sel, sub_btn = me:list(npc, "[칠요구륜]을 만들기 위해서는 모든 요일의 지석 100개가 필요하오. 성공률은 채 일할이 되지 않으니 실패하더라도 양해해 주시오.", { "만들겠습니다.", "아니오... 다시 생각해볼게요." }, { prev = true })
            if sub_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sub_sel == 2 then
                me:dialog(npc, "생각이 바뀌거든 다시 찾아 오시오...", { prev = false, next = false })
                return
            end
            local cost_chil = {}
            for _, name in ipairs(STONES) do
                cost_chil[name] = 100
            end
            local reward = nil
            if math.random(1, 100) < 10 then
                reward = { ['item'] = { ["칠요구륜"] = 1 } }
            end
            local code = me:exchange({ ['item'] = cost_chil }, reward)
            if code == enum.exchange_result.LACK_COST then
                for _, name in ipairs(STONES) do
                    if not me:has_items(name, 100) then
                        me:dialog(npc, "[" .. name .. "]이 부족합니다.", { prev = false, next = false })
                        return
                    end
                end
                me:dialog(npc, "지석이 부족하오.", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 [칠요구륜]을 줄 수 없소.", { prev = false, next = false })
                return
            end
            if reward == nil then
                me:dialog(npc, "이런... [칠요구륜]을 만드는데 실패하고 말았소. 미안하게 됐소...", { prev = false, next = false })
            else
                me:dialog(npc, "[칠요구륜]을 성공적으로 만들었소. 한번 살펴 보시게나....", { prev = false, next = false })
            end
            return
        end

        if sel == 9 then
            local d = me:dialog(npc, "팔세지도는 모든 팔요의 기운이 담긴 칼날형 방패로써, 매우 만들기가 어렵소.\n\n또한 팔요의 기운이 불안정하게 담겨있으므로 팔요지석 100개와 모든 요일지석 100개가 필요하오.", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            local sub_sel, sub_btn = me:list(npc, "[팔세지도]를 만들기 위해서는 모든 요일의 지석 100개와 팔요지석이 필요하오. 성공률은 채 일할이 되지 않으니 실패하더라도 양해해 주시오.", { "만들겠습니다.", "아니오... 다시 생각해볼게요." }, { prev = true })
            if sub_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sub_sel == 2 then
                me:dialog(npc, "생각이 바뀌거든 다시 찾아 오시오...", { prev = false, next = false })
                return
            end
            local cost_pal = { ["팔요지석"] = 100 }
            for _, name in ipairs(STONES) do
                cost_pal[name] = 100
            end
            local reward = nil
            if math.random(1, 100) < 10 then
                reward = { ['item'] = { ["팔세지도"] = 1 } }
            end
            local code = me:exchange({ ['item'] = cost_pal }, reward)
            if code == enum.exchange_result.LACK_COST then
                if not me:has_items("팔요지석", 100) then
                    me:dialog(npc, "[팔요지석]이 부족합니다.", { prev = false, next = false })
                else
                    for _, name in ipairs(STONES) do
                        if not me:has_items(name, 100) then
                            me:dialog(npc, "[" .. name .. "]이 부족합니다.", { prev = false, next = false })
                            return
                        end
                    end
                    me:dialog(npc, "지석이 부족하오.", { prev = false, next = false })
                end
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 [팔세지도]를 줄 수 없소.", { prev = false, next = false })
                return
            end
            if reward == nil then
                me:dialog(npc, "이런... [팔세지도]를 만드는데 실패하고 말았소. 미안하게 됐소...", { prev = false, next = false })
            else
                me:dialog(npc, "[팔세지도]을 성공적으로 만들었소. 한번 살펴 보시게나....", { prev = false, next = false })
            end
            return
        end

        if sel == 10 then
            local d = me:dialog(npc, "팔요천의 옷은 모든 정령의 힘이 궁극으로 합쳐진 옷이오.", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            d = me:dialog(npc, "팔요천의 옷을 만들기 위해서는 [일광방패], [월광방패], [화분천검], [수류곡도], [목근정곤], [금기련봉], [토선투구], [칠요구륜], [팔세지도] 모두를 모두 나에게 재물로 가져와야 하오.", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            d = me:dialog(npc, "또한 현재 성별이 [남성]일 경우 [팔요천의기]를 만들게 되며, 성별이 [여성]일 경우 [팔요천의향]을 만들게 된다오. 이는 음양의 기운에 따른 것이니 바꿀 수 없소.", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            d = me:dialog(npc, ".....그러나 성공률은 일할이 채 되지 않기 때문에, 만들지 않는 것을 추천하오.", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            d = me:dialog(npc, "다만 맡겨준다면 최대한 심혈을 기울여 보겠소.  또한 중간에 허튼짓을 하면 모든 아이템이 날아갈 수 있으므로, 집중하도록 하시오.", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            local sub_sel, sub_btn = me:list(npc, "팔요천의 옷을 만들기 위한 준비가 되었다면, 말을 걸어주시오.", { "만들겠습니다.", "아니오... 다시 생각해볼게요." }, { prev = true })
            if sub_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sub_sel == 2 then
                me:dialog(npc, "생각이 바뀌거든 다시 찾아 오시오...", { prev = false, next = false })
                return
            end
            local need_nine = { "일광방패", "월광방패", "화분천검", "수류곡도", "목근정곤", "금기련봉", "토선투구", "칠요구륜", "팔세지도" }
            local cost_nine = {}
            for _, name in ipairs(need_nine) do
                cost_nine[name] = 1
            end
            local product = (me:gender() == GENDER.MALE) and "팔요천의기" or "팔요천의향"
            local reward = nil
            if math.random(1, 100) < 10 then
                reward = { ['item'] = { [product] = 1 } }
            end
            local code = me:exchange({ ['item'] = cost_nine }, reward)
            if code == enum.exchange_result.LACK_COST then
                for _, name in ipairs(need_nine) do
                    if not me:has_items(name, 1) then
                        me:dialog(npc, "[" .. name .. "]가 부족합니다.", { prev = false, next = false })
                        return
                    end
                end
                me:dialog(npc, "재료가 부족하오.", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 [" .. product .. "]를 줄 수 없소.", { prev = false, next = false })
                return
            end
            if reward == nil then
                me:dialog(npc, "아... 이럴수가... 안타깝게 되었소... 그만 실패하고 말았다오...", { prev = false, next = false })
            else
                me:dialog(npc, "오...! 성공하였군. 당신은 정말 하늘의 축복이 함께하는 사람인가 보오. 정말 축하하오!", { prev = false, next = false })
            end
            return
        end
    end
}
