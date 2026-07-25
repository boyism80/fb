-- npc: 고구려무도가
local quest = require('lib.quest')

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_MARTIAL)
        local btn

        if q == nil then
            ::NPC_175_0001::
            btn = me:dialog(npc, '난 고구려의 무도가요. 얼마전부터 중국의 소림사에 대해 관심을 가지게 됐소. 정말 아무도 그 위치나 규모에 대해서 아는 사람이 없었소.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_175_0002::
            btn = me:dialog(npc, '내 얼마전에 듣기로는 만리장성 부근에 출중한 무술 기량을 가진 사람들이 보인다 하던데. 그들은 모두 승려차림이었다하오.\n\n그들의 절기를 꼭 한번 실제로 보고 배우고싶은데 마땅한 기회가 없구려.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_175_0001
            end
            local sel, lb = me:list(npc, '당신이 날 대신해 중국 만리장성에가서 그들에게 절기를 익혀와 보여주지 않겠소?', { '중국가는길이니 그렇게하겠소.', '난 무술에 별로 관심이 없소.' })
            if lb == DIALOG_RESULT.QUIT then
                return
            end
            if sel == nil or sel ~= 1 then
                return
            end
            q = me:start_quest(quest.QUEST_MARTIAL)
            if q == nil then
                me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
                return
            end
            q:step(1)
            me:push_achievement(26, '고구려무도가의 부탁을 들어주자.', 7, 1)
            me:dialog(npc, '고맙소! 자세한것은 모르지만 요즘 만리장성에 승려차림의 무술고수가 보인다는 소문이 있다오. 부탁하오.', { prev = false, next = true })
            return
        end

        if q:step() == 0 then
            ::NPC_175_0003::
            btn = me:dialog(npc, '난 고구려의 무도가요. 얼마전부터 중국의 소림사에 대해 관심을 가지게 됐소. 정말 아무도 그 위치나 규모에 대해서 아는 사람이 없었소.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_175_0004::
            btn = me:dialog(npc, '내 얼마전에 듣기로는 만리장성 부근에 출중한 무술 기량을 가진 사람들이 보인다 하던데. 그들은 모두 승려차림이었다하오.\n\n그들의 절기를 꼭 한번 실제로 보고 배우고싶은데 마땅한 기회가 없구려.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_175_0003
            end
            local sel, lb = me:list(npc, '당신이 날 대신해 중국 만리장성에가서 그들에게 절기를 익혀와 보여주지 않겠소?', { '중국가는길이니 그렇게하겠소.', '난 무술에 별로 관심이 없소.' })
            if lb == DIALOG_RESULT.QUIT then
                return
            end
            if sel == nil or sel ~= 1 then
                return
            end
            q:step(1)
            me:push_achievement(26, '고구려무도가의 부탁을 들어주자.', 7, 1)
            me:dialog(npc, '고맙소! 자세한것은 모르지만 요즘 만리장성에 승려차림의 무술고수가 보인다는 소문이 있다오. 부탁하오.', { prev = false, next = true })
            return
        end

        if q:step() < 4 then
            local has_baekyeoljang = false
            for _, sp in pairs(me:spells() or {}) do
                if sp:model():name() == '백열장' then
                    has_baekyeoljang = true
                    break
                end
            end
            if not has_baekyeoljang then
                me:dialog(npc, '아직 소림사의 절기를 배워오지 못하신 것 같군요.', { prev = false, next = true })
                return
            end
            ::NPC_175_0005::
            btn = me:dialog(npc, '그래.. 소림사의 절기는 배워오셨소? 오오.. 그 절기의 이름을 백열장이라고 하는군..', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_175_0006::
            btn = me:dialog(npc, '음.. 그렇군. 대충 기술은 이해했소. 나도 무도가라 한번 보면 대략적인 형태는 이해할수 있다오. 이후엔 수련을 해서 더 높은 경지로 갈 수 있도록 해야겠지.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_175_0005
            end
            btn = me:dialog(npc, '이거 고마워서.. 내 뭐라도 줘야할 터인데.. 그렇지. 나의 사부님이 전해주신 손목보호대를 드리지. 뭐 아주 간단한 손목보호대라 다른건 없고, 단단한거 하난 알아주는 물건이오.\n\n그럼 고맙소.. 잘 돌아가시게..', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if me:mkitem('청동손목보호대', 1) == nil then
                me:dialog(npc, '소지품이 가득 차서 ' .. name_with('청동손목보호대', '을', '를') .. ' 줄 수 없네.', { prev = false, next = true })
                return
            end
            q:step(4)
            me:dialog(npc, '고맙소.. 잘 돌아가시게..', { prev = false, next = true })
            return
        end

        me:dialog(npc, '정말 고맙소. 당신 덕분에 나도 소림사의 절기를 더욱더 배워지고 싶구만.', { prev = true, next = true })
    end
}
