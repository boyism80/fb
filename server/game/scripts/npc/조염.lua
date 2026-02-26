local ITEM_BONG_BOOK = '봉래산전설'

function NPC_117(me, npc)
    local quest = me:quest(QUEST_BONG_BOOK)
    local has_book = me:has_items(ITEM_BONG_BOOK, 1)

    if quest == nil then
        local selected = me:list(npc, '내가 책 한권 드릴테니 한번 읽어나 보시게나.', { '예, 주세요.', '아니오, 책이라면 질색이라서..' })
        if selected == nil then
            return
        end
        if selected == 0 then
            if not me:start_quest(QUEST_BONG_BOOK) then
                me:dialog(npc, '퀘스트 시작 실패', false, true)
                return
            end
            if me:mkitem(ITEM_BONG_BOOK, 1) == nil then
                me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', false, true)
                return
            end
            me:dialog(npc, '여기있네. 그럼 재밌게 읽게나.', false, true)
        else
            me:dialog(npc, '그러면 쓰나.. 책을 읽어야 마음에 양식이 쌓이는 것이네.', false, true)
        end
        return
    end

    if not has_book then
        me:dialog(npc, '뭐야? 책을 잃어버려? 이런 고약한.. 내 저번엔 다시 줬지만, 이번엔 그냥은 못주겠네.', true, true)
        local selected = me:list(npc, '1000전을 내면 다시 한권 주지.', { '봉래산전설을 다시 읽고싶어요. 1000전에 주세요.', '돈이 없어서..' })
        if selected == nil or selected ~= 0 then
            return
        end
        local money = me:money()
        if money < 1000 then
            me:dialog(npc, '돈이 부족하지 않나?', false, true)
            return
        end
        if me:mkitem(ITEM_BONG_BOOK, 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', false, true)
            return
        end
        me:money(money - 1000)
        me:dialog(npc, '다시 잘 읽어봐!', false, true)
    else
        me:dialog(npc, '어때? 봉래산전설은 재미있나?', false, true)
    end
end