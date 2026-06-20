local quest = require('lib.quest')

function NPC_95(me, npc)
    local q = me:quest(quest.QUEST_MUTA)
    if q == nil or q:completed() then
        me:dialog(npc, '자네도 이 심상치 않은 기운이 느껴지는가? 요사이, 이곳 저곳에서 불길한 징조가 나타나고 있었다네.', false, true)
        return
    end
    
    local step = q:step()
    if step == 0 then
        ::NPC_95_0001::
        if me:dialog(npc, '자네도 이 심상치 않은 기운이 느껴지는가? 용사이, 이곳 저곳에서 불길한 징조가 나타나고 있었다네. 곳곳의 풍수를 살펴보면 만물이 있어야 할 자리를 지키지 못하고 있으며, 별자리마저 알 수 없는 형태로 일그러져버렸네.', false, true) == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_95_0002::
        local btn = me:dialog(npc, "천기를 살펴본 바로, 이 모든 현상들이 귀결되는 지점이 바로 '반고'의 신전이네. 반고 부활의 조짐일세. 이런 일을 행할 수 있는 이는 오제 뿐이리라네. 신선 세계 일에 관여치 않던 오제가 반고 부활의 의식을 진행하고 있는 것이지. 이 세상은 반고로부터 기인해 존재하게 되었지만, 반고와 현세는 공존할 수 없다네.", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0001
        end
        ::NPC_95_0003::
        btn = me:dialog(npc, '반고의 부활로 인해 새로운 천지가 열리게 될 것이세. 바꾸어 말하자면 곧 현세의 종말을 뜻하는 것일세. 갈수록 더러워지고 추악해지는 인간세상을 정화하려는 신의 분노라고나 할까. 이미 상당히 진행되어 반고의 신격 주변의 생물들은 태고의 본체로 돌아가고 있다네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0002
        end
        ::NPC_95_0004::
        local choice, list_btn = me:list(npc, '정녕 이대로 끝이란 말인가?', {
            '제가 오제의 음모를 막아보겠습니다.',
            '신들이 하는 일을 인간이 어쩔 순 없겠죠.',
        }, true)
        if list_btn == DIALOG_RESULT.QUIT or choice == nil then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_95_0003
        end
        if choice ~= 0 then
            me:dialog(npc, '그렇다면 어쩔 수 없지.', false, true)
            return
        end
        
        ::NPC_95_0005::
        btn = me:dialog(npc, '오, 그래보겠는가? 자네는 이미 상당한 수련을 거쳐 인간의 한계에 근접하고 있는 것으로 보이는군. 물론 신에 대항한다는 것이 쉽진 않겠지만, 내 자네를 믿어보도록 하지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0004
        end
        ::NPC_95_0006::
        btn = me:dialog(npc, '이 일을 해결한다면 자네 한단계 더 높은 경지에 다다를 수 있을 것이야. 지금부터 내가 하는 말을 끝까지 잘 들어보게.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0005
        end
        ::NPC_95_0007::
        btn = me:dialog(npc, "반고의 기운이 점점 강해지고 있는 것으로 보아 부활의 날이 다가오고 있지만, 아직은 희망이 있네. 그것은 오제들이 아직 반고 부활의 의식을 완성하지 못했기 때문일세. 즉 반고 부활의 제물을 찾지 못했다는 것일세. 그들이 그 제물을 찾아내기 전에 저지해야만 하네.", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0006
        end
        ::NPC_95_0008::
        btn = me:dialog(npc, "자네가 해야할 일은 반고의 부활을 꾀하고 있는 오제를 제거하고 증표를 구해오는 것일세. 오제란 소호, 전욱, 제곡, 요제, 순제를 말하지. 먼저 장안성 성곽 부근에 위치한 신전으로 가서, 소호를 물리치고 돌아오게.", true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0007
        end
        
        q:step(1)
        me:push_achievement(7, '소호 암살의 임무를 받다.', 7, 16)
        me:dialog(npc, '먼저 장안성 성곽 부근에 위치한 신전으로 가서, 소호를 물리치고 돌아오게.', false, true)
    end

    if step == 1 then
        if not me:rmitem('소호의증표', 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, '먼저 장안성 성곽 부근에 위치한 신전으로 가서, 소호를 물리치고 돌아오게.', false, true)
            return
        end

        q:step(2)
        ::NPC_95_0009::
        btn = me:dialog(npc, '소호를 잘 물리쳐 주었네. 하지만, 이제 시작이라네.\n이번엔 전욱을 상대해야 하네.\n전욱의 신전이 위치한 곳은 국내성이네.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_95_0010::
        btn = me:dialog(npc, '주의할 점은 임무를 완수하거든 곧장 나에게 돌아와야 한다는 것일세.\n다른 곳에서 전투를 하거나 시간을 지체할 경우 자네가 구한 증표가 효력을 잃을 수 있네.\n시간이 없군. 어서 출발하게!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0009
        end
    end

    if step == 2 then
        if not me:rmitem('전욱의증표', 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, '먼저 국내성에 있는 전욱의 신전으로 가서, 전욱을 물리치고 돌아오게.', false, true)
            return
        end

        q:step(3)
        ::NPC_95_0011::
        btn = me:dialog(npc, '전욱까지 제거하다니, 잘 해주고 있네.\n하지만 아직은 갈 길이 멀다네.\n이번엔 제곡일세. 그의 신전은 부여성에 있다네.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_95_0012::
        btn = me:dialog(npc, '주의할 점은 임무를 완수하거든 곧장 나에게 돌아와야 한다는 것일세.\n다른 곳에서 전투를 하거나 시간을 지체할 경우 자네가 구한 증표가 효력을 잃을 수 있네.\n시간이 없군. 어서 출발하게!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0011
        end
    end

    if step == 3 then
        if not me:rmitem('제곡의증표', 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, '먼저 부여성에 있는 제곡의 신전으로 가서, 제곡을 물리치고 돌아오게.', false, true)
            return
        end

        q:step(4)
        ::NPC_95_0013::
        btn = me:dialog(npc, '제곡을 물리치다니, 대단하네.\n하지만, 다음 번 상대는 더욱 두려운 존재인 요제.\n아직 안심할 때가 아니지.\n요제의 신전은 좀 더 멀다네. 저~기 바다 건너 일본에 있지.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_95_0014::
        btn = me:dialog(npc, '주의할 점은 임무를 완수하거든 곧장 나에게 돌아와야 한다는 것일세.\n다른 곳에서 전투를 하거나 시간을 지체할 경우 자네가 구한 증표가 효력을 잃을 수 있네.\n시간이 없군. 어서 출발하게!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0013
        end
    end

    if step == 4 then
        if not me:rmitem('요제의증표', 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, '먼저 바다 건너 일본에 있는 요제의 신전으로 가서, 요제를 물리치고 돌아오게.', false, true)
            return
        end

        q:step(5)
        ::NPC_95_0015::
        btn = me:dialog(npc, '훌륭하네. 요제까지... 인간으로서 신과 대등하게 싸울 수 있다니. 탄복할 뿐일세.\n이제 마지막 상대는 오제의 수장인 순제, 그는 광동성 신전에서 찾을 수 있을 것이네.\n지금껏 잘 해주었지만, 오제의 우두머리인만큼 결코 만만치 않을 것이야.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_95_0016::
        btn = me:dialog(npc, '주의할 점은 임무를 완수하거든 곧장 나에게 돌아와야 한다는 것일세.\n다른 곳에서 전투를 하거나 시간을 지체할 경우 자네가 구한 증표가 효력을 잃을 수 있네.\n시간이 없군. 어서 출발하게!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0015
        end
    end

    if step == 5 then
        if not me:rmitem('순제의증표', 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, '먼저 광동성에 있는 순제의 신전으로 가서, 순제를 물리치고 돌아오게.', false, true)
            return
        end

        q:step(6)
        ::NPC_95_0017::
        btn = me:dialog(npc, '대단하네! 드디어 오제를 모두 멸하였군.\n휴우~ 이것으로 끝난 것인가, 이제 반고의 기운이 사라지겠지...?!\n아니, 이것은? 이것이 어떻게 된 일이란 말인가?\n반고의 기운이 이토록 온 천지를 덮고 있다니... 어째서 이토록 증폭되고 있단 말인가!', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_95_0018::
        btn = me:dialog(npc, '오~ 이럴수가... 바로 그것이었나? 그때문에 오제들이 그토록 담담히 최후를 맞이하였던가!\n오제를 자신이 바로 반고 부활의 제물이었단 말인가...!\n그들이 하지 못했던 것은 제물을 찾는 것이 아니라, 그 제물을 바치는 것이었어.\n하지만, 삶에 대한 인간의 욕심이 스스로를 종말로 밀어넣을 것을 그들은 알고 있었던 게야.\n결국 우리 인간은 신들의 예언대로 자멸하고 마는군. 으흑흑!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0017
        end
        ::NPC_95_0019::
        local choice2, list_btn = me:list(npc, '그렇다면 자네는 어떻게 하겠나?', {
            '이 땅이 아직 건재한데, 포기하긴 이릅니다.',
            '만고를 제거해야 하지 않겠습니까?',
        }, true)
        if list_btn == DIALOG_RESULT.QUIT or choice2 == nil then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_95_0018
        end
        ::NPC_95_0020::
        btn = me:dialog(npc, '자네의 기개가 대단하군. 그래, 아직은 아니지!\n반고는 인간이 감히 상대할 수 있는 존재가 아니지만, 이제 막 부활하여 그 기운을 완전히 되찾진 못했을 것이야.\n자네에게 이런 짐을 지우는 것이 못내 안쓰럽네만, 기회는 지금 뿐일세.\n어서, 광동성 신전으로 돌아가 반고를 물리치고 그 심장을 구해오게!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0019
        end
        ::NPC_95_0021::
        btn = me:dialog(npc, '주의할 점은 임무를 완수하거든 곧장 나에게 돌아와야 한다는 것일세.\n다른 곳에서 전투를 하거나 시간을 지체할 경우 자네가 구한 증표가 효력을 잃을 수 있네.\n시간이 없군. 어서 출발하게!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0020
        end
    end

    if step == 6 then
        if not me:has_items('반고의심장', 1) then
            me:dialog(npc, '먼저 광동성 신전에서 반고를 물리치고 그 심장을 구해오게.', false, true)
            return
        end

        q:step(7)
        ::NPC_95_0022::
        btn = me:dialog(npc, '자네를 다시 보게 되어 반가우면서도 놀랍기 그지 없네 그려.\n인간으로서 반고를 제압할 수 있는 자가 있을 줄은 나로선 상상하지도 못했었네.\n이로서, 멸절의 위기를 넘겼으니 기쁘긴 하네만, 우리가 추악한 욕심을 버리고 세상을 스스로 정화하지 못한다면 언젠가 이런 일이 다시 생길 수 있을 것이야.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_95_0023::
        btn = me:dialog(npc, '반고의 심장을 들고 천상계의 천성랑을 찾아가게.\n이번 일로 최고의 경지에 오른 것을 증명한 자네인만큼, 그에 걸맞는 새로운 지위를 인정받을 수 있을 것이네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_95_0022
        end
    end
end
