local quest = require('lib.quest')
local server = require('lib.server')
local function do_jungki(me, npc)
    local q = me:quest(quest.QUEST_SHARK_WEAPON)

    if q  == nil then
    end

    local step = q:step()
    if step < 4 then
        me:dialog(npc, '자네는 아직 알 때가 아니군.', false, false)
        return
    end
    
    if step == 4 then
        local sel = me:list(npc, '무슨 일인가?', { '용궁의정기를 만들줄 아십니까?' }, false)
        if sel == nil or sel ~= 0 then
            return
        end
        if me:list(npc, '물론 만들줄은 아네만...그리 쉽게 만들수는 없다네..', { '!!!' }, false) ~= 0 then
            return
        end
        ::NPC_157_0000::
        if me:list(npc, '그래도 만들고 싶은가?', { '물론이지요.', '아니요. 포기할래요.' }, false) ~= 0 then
            return
        end
        ::NPC_157_0001::
        local d_btn = me:dialog(npc, '한번에 다 알려줘 봐야 소용없으니 한번에 하나씩만 알려주겠네.', true, true)
        if d_btn == DIALOG_RESULT.QUIT then
            return
        end
        if d_btn == DIALOG_RESULT.PREV then
            goto NPC_157_0000
        end
        if me:dialog(npc, '먼저 복어의심장 30개를 가지고 오게나.', false, false) == DIALOG_RESULT.QUIT then
            return
        end
        if q then
            q:step(5)
        end
        me:push_achievement(24, '무달의 부탁을 들어주자. [1/5]', 7, 1)
        return
    end
    
    local steps = {
        { step_in = 5, item = '복어의심장', count = 30, next_msg = '그럼 이젠 문어의심장을 30개 모아오게나.', reward = '용궁의정기1', legend = '무달의 부탁을 들어주자. [2/5]' },
        { step_in = 6, item = '문어의심장', count = 30, next_msg = '그럼 이젠 해마의심장을 30개 모아오게나.', reward = '용궁의정기2', legend = '무달의 부탁을 들어주자. [3/5]' },
        { step_in = 7, item = '해마의심장', count = 30, next_msg = '그럼 이젠 인어의심장을 30개 모아오게나.', reward = '용궁의정기3', legend = '무달의 부탁을 들어주자. [4/5]' },
        { step_in = 8, item = '인어의심장', count = 30, next_msg = '그럼 이젠 상어의심장을 30개 모아오게나.', reward = '용궁의정기4', legend = '무달의 부탁을 들어주자. [5/5]' },
        { step_in = 9, item = '상어의심장', count = 30, next_msg = nil, reward = '용궁의정기5', legend = '무달의 부탁을 들어주었다.' },
    }
    for _, s in ipairs(steps) do
        if step == s.step_in then
            local code = me:exchange(
                { ['item'] = { [s.item] = s.count } },
                { ['item'] = { [s.reward] = 1 } }
            )
            if code == server.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, s.item .. ' 갯수가 부족한 것은 아닌가? 30개가 필요하네.', false, false)
                return
            end
            if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 ' .. name_with(s.reward, '을', '를') .. ' 받을 수 없네.', false, false)
                return
            end
            if q then
                q:step(s.step_in + 1)
            end
            me:push_achievement(24, s.legend, 7, 1)
            if s.next_msg then
                me:dialog(npc, '다 모아왔군 그래. ' .. s.next_msg, false, false)
            else
                me:dialog(npc, '이제 용궁의정기는 모두 다 모았네. 가서 볼일을 보시게나.', false, false)
            end
            return
        end
    end
    if step >= 10 then
        me:dialog(npc, '......', false, false)
    end
end

local function do_armor_infinite(me, npc)
    local price = 1000
    local pay_sel = me:list(npc, string.format('무한에 참여하시려면 염색비 %d전을 내시고 무한 복장을 하셔야 합니다. %d전을 내시겠습니까?', price, price), { '예', '아니오' })
    if pay_sel == nil then
        return
    end
    if pay_sel ~= 0 then
        return
    end
    local money = me:money()
    if money < price then
        me:dialog(npc, '돈이 모자랍니다.')
        return
    end
    local team_sel = me:list(npc, '어떤 팀으로 참가하시겠습니까?', { '북팀 - 검은색', '남팀 - 붉은색', '동팀 - 파란색', '서팀 - 하얀색' })
    if team_sel == nil then
        return
    end
    if team_sel == 0 then
        me:armor_color(10)
    elseif team_sel == 1 then
        me:armor_color(31)
    elseif team_sel == 2 then
        me:armor_color(17)
    elseif team_sel == 3 then
        me:armor_color(11)
    end
    me:money(money - price)
    ::NPC_157_0002::
    if me:dialog(npc, string.format('염색비로 %d전을 받았습니다.', price), false, true) == DIALOG_RESULT.QUIT then
        return
    end
    local last_btn = me:dialog(npc, '그럼 언제나 행운이 함께 하시기를 빌겠습니다.', true, true)
    if last_btn == DIALOG_RESULT.QUIT then
        return
    end
    if last_btn == DIALOG_RESULT.PREV then
        goto NPC_157_0002
    end
end

local function do_armor_normal(me, npc)
    local sel = me:list(npc, '일반 복장으로 바꿔드릴까요?', { '예', '아니오' })
    if sel == nil then
        return
    end
    if sel ~= 0 then
        return
    end
    me:armor_color(nil)
    me:dialog(npc, '일반 복장으로 바꿔드렸습니다.', false, true)
end

function NPC_157(me, npc)
    local selected = me:list(npc, '안녕하세요. 어떻게 오셨나요?', { '정기모으기', '무한 복장', '일반 복장' }, false)
    if selected == nil then
        return
    end
    if selected == 0 then
        do_jungki(me, npc, q, step)
        return
    end
    if selected == 1 then
        do_armor_infinite(me, npc)
        return
    end
    if selected == 2 then
        do_armor_normal(me, npc)
        return
    end
end
