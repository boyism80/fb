-- npc: 성원왕
local quest = require('lib.quest')
local enum = require('lib.enum')
local function do_sub6_start(me, npc)
    local sel, btn = me:list(npc, "요즘따라 인간들을 자주 만나시게 되시는도다.", { "이름을 가르쳐주세요.", "이곳의 상황은 좀 어떤가요?", "제가 도와드릴 일은 없을까요?" }, { prev = false })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return false
    end
    if sel == 1 then
        me:dialog(npc, "나의 이름은 알아서 뭐하시려고 그러시는가?", { prev = false, next = false })
        return true
    end
    if sel == 2 then
        me:dialog(npc, "이 곳의 상황은 본래와 다를바 없으시다.", { prev = false, next = false })
        return true
    end
    sel, btn = me:list(npc, "음? 하하하. 고마운 말씀이시다. 진심이신가?", { "네, 꼭 도와드릴께요.", "아뇨, 그만 둘래요." }, { prev = true })
    if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
        me:dialog(npc, "장난을 치는 사람은 싫어한다.", { prev = false, next = false })
        return true
    end
    local b = me:dialog(npc, "이 땅에는 호박이라는 아주 예쁜 물건이 있으시다고 들으셨다. 병에 걸리신 원숭이들 때문에", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "마음에 상처를 입으신 우리 원숭이들을 위로해주실 수 있으실지도 모르신다. 그걸 자네가 좀", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "구해주시길 원하신다. 그래 한 80개면 충분하실거다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    local q6 = me:quest(quest.QUEST_SKULL_NECKLACE_6)
    if q6 == nil then
        q6 = me:start_quest(quest.QUEST_SKULL_NECKLACE_6)
        if q6 == nil then
            return true
        end
    end
    if q6 then
        q6:step(1)
    end
    return true
end

local function do_sub6_turnin(me, npc)
    if not me:has_items("호박", 80) then
        me:dialog(npc, "한 호박 80개면 충분하실거다.", { prev = false, next = false })
        return true
    end
    local b = me:dialog(npc, "굉장히 예쁜 물건이시다! 호박이라는게 이렇게 예쁘실 줄이야! 모두에게 나누어드리면 시름을 잊으시고", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "들여다 보실거다! 고마우시다! 아주 수고하셨다! 이건 내가 몰래 숨겨가지고 계시던 간식이시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    local code = me:exchange(
        { ['item'] = { ["호박"] = 80 } },
        { ['item'] = { ["바다거북의알"] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
        return true
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 바다거북의알을 줄 수 없습니다.", { prev = false, next = false })
        return true
    end
    local q6 = me:quest(quest.QUEST_SKULL_NECKLACE_6)
    if q6 then
        q6:step(2)
    end
    return true
end

function NPC_468(me, npc)
    local q6 = me:quest(quest.QUEST_SKULL_NECKLACE_6)

    if q6 == nil or q6:step() == 0 then
        if do_sub6_start(me, npc) then
            return
        end
        return
    end

    if q6:step() == 1 then
        if do_sub6_turnin(me, npc) then
            return
        end
        return
    end

    local main_q = me:quest(quest.QUEST_SKULL_NECKLACE)
    if main_q then
        local s = main_q:step()
        if s == 20 then
            local b = me:dialog(npc, "참원왕이 의견을 물어 여기까지 왔으시라고?", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            b = me:dialog(npc, "인간들은 음식을 구워서 드신다며? 잘은 모르시지만, 우리도 음식을 구워 드시면 병에 걸리시는 일이", { prev = true, next = true })
            if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
                return
            end
            b = me:dialog(npc, "없으실 것만 같으시다. 그 횃불이라고 부르시는 물건을 구하셔서 구워 드셔보는게 어떠실까 하신다.", { prev = true, next = true })
            if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
                return
            end
            main_q:step(21)
            return
        end
        if s == 21 then
            me:dialog(npc, "내 의견을 참원왕에게 어서 전달해주셨으면 좋으시겠다.", { prev = false, next = false })
            return
        end
        if s == 30 then
            if not me:has_items("마른갈대", 1) then
                me:dialog(npc, "마른갈대를 구해서 왕들에게 하나씩 나누어 주게.", { prev = false, next = false })
                return
            end
            local b = me:dialog(npc, "마른 갈대를 나눠주고 있다고 들었네. 수고하는 모습이 참 보기 좋군. 더 수고해주게.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            if not me:rmitem("마른갈대", 1, ITEM_DELETE_TYPE.GIVE) then
                me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                return
            end
            main_q:step(31)
            return
        end
    end

    me:dialog(npc, "준비중입니다.", { prev = false, next = false })
end