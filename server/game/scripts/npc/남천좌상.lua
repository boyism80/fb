function NPC_236(me, npc)
    local BONE_COUNT = 500
    local quest = me:quest(QUEST_HOO_KILL)

    if quest == nil then
        ::NPC_236_0001::
        local btn = me:dialog(npc, '요즘 백성들이 후를 두려워해서 집밖으로 다니는것 조차 두려워하고 있다네. 일정한 곳에 머물지 않고 땅을 파고 어디든지 돌아다니기 때문에 더욱 두려운 존재야.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_236_0002::
        btn = me:dialog(npc, '이놈들을 퇴치하지 않으면 머지않아 우리나라가 쑥밭이 될 지경이니 이거 정말 큰일이네.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_236_0001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '자네가 후를 퇴치하여 주지 않겠나? 제발 부탁하네.', { '그러죠. 제가 처리하겠습니다!!', '저보다 나은 적임자가 분명 있을겁니다.' }, false)
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_236_0003::
        btn = me:dialog(npc, '고맙네. 후는 번식력도 강해서 한번에 ' .. BONE_COUNT .. '마리 이상 잡지 않으면 금방 다시 번식을 한다네. 후는 여러종류가 있지만, 어떤 종류든 후를 잡으면 뼈를 남긴다네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_236_0004::
        btn = me:dialog(npc, '자네가 후를 ' .. BONE_COUNT .. '마리 이상 잡았다는 증거로, ' .. BONE_COUNT .. '마리의 뼈를 가지고 오면 내 포상을 하지. \'후의뼈\' ' .. BONE_COUNT .. '개라네.', true, false)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_236_0003
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        quest = me:start_quest(QUEST_HOO_KILL)
        if quest == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest:step(1)
        me:push_achievement(38, '후 말살 퀘스트를 받다.', 7, 1)
        return
    end

    if quest:completed() then
        me:dialog(npc, me:name() .. '씨 아니오? 저번엔 정말 감사했소. 덕분에 백성들의 고민을 한결 덜어주었다네.', false, true)
        return
    end

    if quest:step() == 1 then
        local btn
        if not me:has_items('후의뼈', BONE_COUNT) then
            me:dialog(npc, '후의뼈가 너무 적은것은 아닌가? ' .. BONE_COUNT .. '마리 이상을 잡고 후의뼈 ' .. BONE_COUNT .. '개를 가져와주게.', false, true)
            return
        end
        ::NPC_236_0005::
        btn = me:dialog(npc, BONE_COUNT .. '마리를 잡는데 성공했군! 하지만 아직 일이 끝난게 아닐세. 미리 얘기를 하지 않았지만 후라는 녀석은 마계천신이라는 왕이 있어서 그 왕이 사라지지 않는 이상 시간이 지나면 더욱 강하게 되살아나지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_236_0006::
        btn = me:dialog(npc, '그래서 한가지 부탁을 더 해야겠네. 마계천신을 잡아 그의 뼈를 가져와주게. 정말 마지막 부탁이네.', true, false)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_236_0005
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '마계천신의뼈를 가져와줄 수 있겠나?', { '물론이죠! 다시는 되살아나지 못하도록 해야합니다.', '그런것까지 제가 할 필요는 없는것 같군요.' }, false)
        if sel == nil or sel ~= 0 then
            return
        end
        quest:step(2)
        me:rmitem('후의뼈', BONE_COUNT, ITEM_DELETE_TYPE.GIVE)
        me:push_achievement(38, '마계천신의뼈를 구하자.', 7, 1)
        ::NPC_236_0007::
        btn = me:dialog(npc, '오, 고맙네. 그럼 자네가 가지고온 후의뼈는 가져가도록 하겠네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_236_0008::
        btn = me:dialog(npc, '그럼 마저 부탁하지. 행운을 비네.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_236_0007
        end
        return
    end

    if quest:step() == 2 then
        local btn
        if not me:has_items('마계천신의뼈', 1) then
            me:dialog(npc, '아직 마계천신의뼈를 구하지 못한 모양이로군..', false, true)
            return
        end
        ::NPC_236_0009::
        btn = me:dialog(npc, '오오, 정말로 마계천신의뼈를 가지고 왔군. 자넨 정말 대단한 인물이군..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_236_0010::
        btn = me:dialog(npc, '정말 진심으로 고맙네. 자넨 정말 큰일을 해냈어! 허허...', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_236_0009
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_236_0011::
        btn = me:dialog(npc, '이건 고마움의 표시니 가져가도록 하게. 고구려에 있다는 숨겨진 마을에서 만들었다는 \'세시투구\'를 흉내내어 만들어 본 것이라네. 물론 세시투구만큼의 성능이 나오진 않지만 말이야.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_236_0010
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '이 투구를 쓰고 있으면 자네의 무기와 갑옷의 내구력이 오랫동안 유지될걸세. 그럼 잘 가시게..', true, false)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_236_0011
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local code = me:exchange(
            { ['item'] = { ['마계천신의뼈'] = 1 } },
            { ['item'] = { ['강철투구'] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '마계천신의뼈를 가지고 있지 않으시군요.', false, true)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 강철투구를 받을 수 없습니다.', false, true)
            return
        end
        quest:complete()
        me:push_achievement(38, '후 말살 임무를 완수하였다.', 6, 1)
        return
    end
end