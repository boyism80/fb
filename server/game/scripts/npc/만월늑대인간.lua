-- @note Trash: 1_일본\일반.txt "만월늑대인간". Linear story with @pass 1-11: list/dialog chain; pass 11: require 해골왕의뼈, 유성지의보패, 철광석, 불의수정 1 each, delitem, deletenpc + mobspawn. FB: QUEST_WOLF_CURSE step = pass; hand-in with has_items/rmitem; npc:destroy() + me:spawn_mob("만월늑대인간", x, y, false) at NPC position.

local REQUIRED_ITEMS = {
    { "해골왕의뼈", 1 },
    { "유성지의보패", 1 },
    { "철광석", 1 },
    { "불의수정", 1 },
}

--- @brief NPC 만월늑대인간 (노정월): Cursed werewolf story; collect proof items and hand in (mobspawn omitted).
--- @param me   character talking to the NPC
--- @param npc  NPC entity (만월늑대인간)
function NPC_549(me, npc)
    local quest = me:quest(QUEST_WOLF_CURSE)
    if quest and quest:completed() then
        me:dialog(npc, "증표는 이미 받았다. 의식이 준비되면 알려주겠지.", false, false)
        return
    end
    local step = (quest and quest:step()) or 0

    if step == 0 then
        local sel, list_btn = me:list(npc, "이 곳은 저주받은자들이 기거하는 곳...", { "당신은 누구신가요?" }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            quest = me:start_quest(QUEST_WOLF_CURSE)
            if quest == nil then
                me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
                return
            end
            if quest then
                quest:step(1)
            end
        end
        return
    end

    if step == 1 then
        local sel, list_btn = me:list(npc, "내 이름은 노정월. 저주를 받아 늑대의 형상을 하고 있다네.", { "올라오는 길에 살펴보니,", "다른 비슷한 늑대인간이 있던데요." }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(2)
        end
        return
    end

    if step == 2 then
        local sel, list_btn = me:list(npc, "그렇지. 그들 모두 저주를 받아, 음의 기운이 가득한 보름달이 뜨는 날에만 그들은 모두 제 정신을 찾게 된다네.", { "당신은 어떻게 저와 얘기를 나눌 수 있죠?" }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(3)
        end
        return
    end

    if step == 3 then
        local sel, list_btn = me:list(npc, "양의기운이 가장 강한 장소인 이 곳은 나의 저주를 억눌러 주고 있지. 덕분에 내가 인간일 때의 마음을 가질 수 있게 되었지...", { "그런 일이..." }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(4)
        end
        return
    end

    if step == 4 then
        local sel, list_btn = me:list(npc, "이 곳에서 미쳐 있는 그들은 보름달이 뜨는 날 밤, 단 하루만 제 정신을 유지할 수 있다네.", { "저런, 안타까운 일이로군요." }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(5)
        end
        return
    end

    if step == 5 then
        local sel, list_btn = me:list(npc, "이 곳을 감싸는 불의 기운은 양의 기운으로 가득하고, 보름달이 뜨는 날은 음의 기운이 세상을 덮는다네.", { "음의기운... 양의기운...", "어려운 말이로군요." }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(6)
        end
        return
    end

    if step == 6 then
        local sel, list_btn = me:list(npc, "그 둘의 기운이 완벽하게 조화를 이루는 시점에야 우리같은 저주를 받은 자들이 인간의 마음을 가질 수 있게 되는 것이지.", { "그런 불행한 일이..." }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(7)
        end
        return
    end

    if step == 7 then
        local sel, list_btn = me:list(npc, "그런데... 최근들어 저주받은 자, 망령된 자들이 저주의 기운을 더욱 강화 시키고 있다네. 조금이나마 저주가 덜어진다면 바랄게 없겠지.", { "제가 도와드릴 일이라도?" }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(8)
        end
        return
    end

    if step == 8 then
        local sel, list_btn = me:list(npc, "악귀가 되어버린 해골왕, 귀기 서린 유성지, 그리고 귀신에 씌인 수도승들... 이미 영면에 처했어야 할 존재들이라네.", { "신궁에서 보았던 자 들이로군요..." }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(9)
        end
        return
    end

    if step == 9 then
        local sel, list_btn = me:list(npc, "그들을 쓰러뜨리고, 그들의 증표를 가져와 주게나. 그들의 귀기가 사라진다면 한결 낫겠지. 그리고 나의 기운을 담아 낼 강한 금속이 필요하다네.", { "알겠습니다. 무엇을 가져오면 될까요?" }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if quest then
            quest:step(10)
        end
        return
    end

    if step == 10 then
        local btn = me:dialog(npc, "해골왕의 증표인 [해골왕의뼈], 그리고 유성지의 증표인 [유성지의보패], 그리고 내 기운을 담아 낼 금속인 [철광석], 마지막으로 이를 한데 섞기 위한 [불의수정]이 필요하다네.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if quest then
            quest:step(11)
        end
        return
    end

    -- step == 11: hand-in
    for i = 1, #REQUIRED_ITEMS do
        local name, count = REQUIRED_ITEMS[i][1], REQUIRED_ITEMS[i][2]
        if not me:has_items(name, count) then
            me:dialog(npc, name .. "가 없군...", false, false)
            return
        end
    end

    local sel, list_btn = me:list(npc, "모든 증표를 가져오게나. 자네의 자격을 시험하겠네.", { "의식을 시작할 준비가 되었습니다." }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel ~= 0 then
        return
    end

    for i = 1, #REQUIRED_ITEMS do
        local name, count = REQUIRED_ITEMS[i][1], REQUIRED_ITEMS[i][2]
        if not me:rmitem(name, count, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, name .. "를 건네지 못했습니다. 인벤을 확인해 주세요.", false, false)
            return
        end
    end

    if quest then
        quest:completed()
    end
    local x, y = npc:position()
    me:dialog(npc, "증표를 잘 받았다. 의식은 이곳의 기운이 맞을 때 진행되지.", false, false)
    npc:destroy()
    me:spawn_mob("만월늑대인간", x, y, false)
end
