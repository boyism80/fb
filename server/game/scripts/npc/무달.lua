function NPC_157(me, npc)
    local q = me:quest(QUEST_SHARK_WEAPON)
    local step = (q and q:step()) or 0
    if step >= 4 then
        local menu_sel = me:list(npc, '안녕하세요. 어떻게 오셨나요?', { '정기모으기', '무한 복장', '일반 복장' }, false)
        if menu_sel == nil then
            return
        end
        if menu_sel == 0 then
            local function run_jungki()
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
                    if me:list(npc, '그래도 만들고 싶은가?', { '물론이지요.', '아니요. 포기할래요.' }, false) ~= 0 then
                        return
                    end
                    if me:dialog(npc, '한번에 다 알려줘 봐야 소용없으니 한번에 하나씩만 알려주겠네.', true, true) == DIALOG_RESULT.QUIT then
                        return
                    end
                    if me:dialog(npc, '먼저 복어의심장 30개를 가지고 오게나.', true, false) == DIALOG_RESULT.QUIT then
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
                        if not me:has_items(s.item, s.count) then
                            me:dialog(npc, s.item .. ' 갯수가 부족한 것은 아닌가? 30개가 필요하네.', false, false)
                            return
                        end
                        if not me:rmitem(s.item, s.count, ITEM_DELETE_TYPE.GIVE) then
                            return
                        end
                        if q then
                            q:step(s.step_in + 1)
                        end
                        me:mkitem(s.reward, 1)
                        me:push_achievement(24, s.legend, 7, 1)
                        if s.next_msg then
                            me:dialog(npc, '다 모아왔군 그래. ' .. s.next_msg, true, false)
                        else
                            me:dialog(npc, '이제 용궁의정기는 모두 다 모았네. 가서 볼일을 보시게나.', true, false)
                        end
                        return
                    end
                end
                if step >= 10 then
                    me:dialog(npc, '......', true, false)
                end
            end
            run_jungki()
            return
        end
        if menu_sel == 1 then
            goto ROUTINE_ARMOR_0
        end
        if menu_sel == 2 then
            goto ROUTINE_ARMOR_1
        end
        return
    end

::ROUTINE_CHANGE_ARMOR_COLOR_0::
    local index, button = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {'무한 복장', '일반 복장'})
    if index == nil then
        return
    end

::ROUTINE_ARMOR_MERGE::
::ROUTINE_ARMOR_0::
    if index ~= 0 and index ~= 1 then
        index = 0
    end
    if index == 0 then
        local price = 1000
        index = me:list(npc, string.format('무한에 참여하시려면 염색비 %d전을 내시고 무한 복장을 하셔야 합니다. %d전을 내시겠습니까?', price, price), {'예', '아니오'})
        if index == nil then
            return
        end

        if index ~= 0 then
            return
        end

        index = me:list(npc, '어떤 팀으로 참가하시겠습니까?', {'북팀 - 검은색', '남팀 - 붉은색', '동팀 - 파란색', '서팀 - 하얀색'})
        if index == nil then
            return
        end

        if index == 0 then
            me:armor_color(10);
        elseif index == 1 then
            me:armor_color(31);
        elseif index == 2 then
            me:armor_color(17);
        elseif index == 3 then
            me:armor_color(11);
        else
        end

        local money = me:money()
        if money < price then
            me:dialog(npc, '돈이 모자랍니다.')
            return
        end

::ROUTINE_CHANGE_ARMOR_COLOR_1::
        me:money(money - price)
        if me:dialog(npc, string.format('염색비로 %d전을 받았습니다.', price), false, true) == DIALOG_RESULT.QUIT then
            return
        end

        button = me:dialog(npc, '그럼 언제나 행운이 함께 하시기를 빌겠습니다.', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if button == DIALOG_RESULT.PREV then
            goto ROUTINE_CHANGE_ARMOR_COLOR_1
        end

        goto ROUTINE_CHANGE_ARMOR_COLOR_0
    end

::ROUTINE_ARMOR_1::
    index = 1
    if index == 1 then
    	index = me:list(npc, '일반 복장으로 바꿔드릴까요?', {'예', '아니오'})
    	if index == nil then
    		return
    	end

    	if index ~= 0 then
    		return
    	end

    	me:armor_color(nil)
    	button = me:dialog(npc, '일반 복장으로 바꿔드렸습니다.', false, true)
    	if button == DIALOG_RESULT.QUIT then
    		return
    	end
    	
    	goto ROUTINE_CHANGE_ARMOR_COLOR_0
    end
end