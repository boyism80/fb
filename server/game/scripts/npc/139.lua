-- npc: 천금장인
local enum = require('lib.enum')

local DRAGON_LOW_LINES = {
    { type_name = '용마', check_items = { '용마제일검', '용마제이검', '용마제삼검', '용마제사검', '용마제오검' }, success_items = { '용마제이검', '용마제삼검', '용마제사검', '용마제오검', '용마제육검' } },
    { type_name = '용천', check_items = { '용천제일검', '용천제이검', '용천제삼검', '용천제사검', '용천제오검' }, success_items = { '용천제이검', '용천제삼검', '용천제사검', '용천제오검', '용천제육검' } },
    { type_name = '용랑', check_items = { '용랑제일봉', '용랑제이봉', '용랑제삼봉', '용랑제사봉', '용랑제오봉' }, success_items = { '용랑제이봉', '용랑제삼봉', '용랑제사봉', '용랑제오봉', '용랑제육봉' } },
    { type_name = '용겸', check_items = { '용겸제일봉', '용겸제이봉', '용겸제삼봉', '용겸제사봉', '용겸제오봉' }, success_items = { '용겸제이봉', '용겸제삼봉', '용겸제사봉', '용겸제오봉', '용겸제육봉' } },
}

local SUCCESS_RATES = { 85, 70, 50, 40, 25 }

return {
    on_click = function(me, npc)
        ::NPC_139_0000::
        local OPT_AWAKEN = '하급 용무기 각성'
        local sel, btn = me:pursuit(npc, '안녕하세요. 어떻게 오셨나요?', { OPT_AWAKEN })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= OPT_AWAKEN then
            return
        end

        ::NPC_139_0001::
        local d = me:dialog(npc, '흐음.. 자네는 전설의 무기인 용무기를\n각성시키고자 하는가?\n\n그렇다면 잘 찾아왔네.\n\n허나, 7등급 이상의 상급 용무기를 각성시키려 한다면 전설의 대장장이로 일컬어지는 백의장인을 찾아야 한다네.\n\n나에게 하급 용무기 각성을 가르쳐주신 위대한 분이시지.', { prev = true, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        if d == DIALOG_RESULT.PREV then
            goto NPC_139_0000
        end

        ::NPC_139_0002::
        d = me:dialog(npc, '용무기는 각성을 통해 한 단계씩 성장하게 된다는 사실을 자네는 이미 알고 있는 듯 하군.\n\n하지만 이는 무척 시도하기 쉽지 않은 일이라네.\n\n만약 실패할 경우 그 귀한 용무기가 부숴저버릴수도 있으니..', { prev = true, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        if d == DIALOG_RESULT.PREV then
            goto NPC_139_0001
        end

        ::NPC_139_0003::
        d = me:dialog(npc, '7등급 이상의 완숙한 용무기들은 각성이 실패해도 부숴지는 일은 없네만, 손상을 입어 더이상 각성이 불가능한 상태가 되어버리지.\n손상된 용무기는 신수성에서의 수리비도 5배나 더 비싸진다네.\n\n게다가 각성을 위해서는 특별한 재료를 필요로 하지.\n용무기는 지극히 높은 온도에서 제련을 통해 발현된 용의 기운을 타고 각성하게 되는데, 충분한 온도를 내기 위해선 \'은나무가지\'가 필요하다네.', { prev = true, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        if d == DIALOG_RESULT.PREV then
            goto NPC_139_0002
        end

        ::NPC_139_0004::
        local confirm_sel, confirm_btn = me:list(npc, '사람이 하는 일에 완전은 있을 수 없다네. 용의 기운은 미묘하여, 하늘, 땅, 그리고 사람의 기운중 하나가 조금이라도 어긋나면 용무기는 깨져버릴 것이네. 자네는 실패의 위험을 무릎쓰고서 용무기를 각성시키려는가?', { '예, 진인사재천명! 하늘에 뜻에 맡기겠습니다.', '아니요, 내 귀한 용무기를 시험에 들게할 순 없죠.' }, { prev = true })
        if confirm_btn == DIALOG_RESULT.QUIT then
            return
        end
        if confirm_btn == DIALOG_RESULT.PREV then
            goto NPC_139_0003
        end
        if confirm_sel == nil then
            return
        end
        if confirm_sel == 2 then
            me:dialog(npc, '그래, 자네 판단이 맞을수도 있을걸세.\n\n나도 나 자신을 믿을수 없으니말일세.\n\n그럼 가보게나.', { prev = false, next = false })
            return
        end
        if confirm_sel ~= 1 then
            return
        end

        ::NPC_139_0005::
        local type_opts = {}
        for _, line in ipairs(DRAGON_LOW_LINES) do
            type_opts[#type_opts + 1] = line.type_name
        end
        local type_sel, type_btn = me:list(npc, '좋아 각오가 대단하군.\n그럼 어떤 용무기를 각성시키고자 하는가?', type_opts, { prev = true })
        if type_btn == DIALOG_RESULT.QUIT then
            return
        end
        if type_btn == DIALOG_RESULT.PREV then
            goto NPC_139_0004
        end
        if type_sel == nil or type_sel < 1 or type_sel > #DRAGON_LOW_LINES then
            return
        end

        local line = DRAGON_LOW_LINES[type_sel]
        local grade_sel, grade_btn = me:list(npc, '각성시킬 용무기를 고르게', line.check_items, { prev = false })
        if grade_btn == DIALOG_RESULT.QUIT then
            return
        end
        if grade_btn == DIALOG_RESULT.PREV then
            goto NPC_139_0005
        end
        if grade_sel == nil or grade_sel < 1 or grade_sel > #line.check_items then
            return
        end

        local check_item = line.check_items[grade_sel]
        local success_item = line.success_items[grade_sel]
        local rate = SUCCESS_RATES[grade_sel]

        local cost = { ['item'] = { ['은나무가지'] = 1, [check_item] = 1 } }
        local roll = math.random(1, 100)
        local reward = (roll <= rate) and { ['item'] = { [success_item] = 1 } } or nil
        local code = me:exchange(cost, reward)
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, '자네는 아직 용무기를 각성시킬 준비가 완벽하지 못하군.', { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(success_item, '을', '를') .. ' 받을 수 없군.', { prev = false, next = false })
            return
        end
        if reward ~= nil then
            broadcast(string.format('%s님이 %s 강화에 성공하셨습니다.', me:name(), check_item), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
            me:dialog(npc, '축하하네. 하늘이 자네를 ' .. success_item .. '의 주인으로 인정하는군.', { prev = false, next = true })
        else
            broadcast(string.format('%s님이 %s 강화에 실패하셨습니다.', me:name(), check_item), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
            me:dialog(npc, '미안하네. 온도 조절이 실패하여 용무기가 부숴저버렸군.. 너무 낙심하지 말게나.', { prev = false, next = false })
        end
    end
}
