-- npc: 금원왕
local quest = require('lib.quest')
local enum = require('lib.enum')
local function do_sub8_start(me, npc)
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
    ::NPC_470_0009::
    sel, btn = me:list(npc, "음? 하하하. 고마운 말씀이시다. 진심이신가?", { "네, 꼭 도와드릴께요.", "아뇨, 그만 둘래요." }, { prev = true })
    if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
        me:dialog(npc, "장난을 치는 사람은 싫어한다.", { prev = false, next = false })
        return true
    end
    ::NPC_470_0010::
    local b = me:dialog(npc, "고마우시다. 내가 아직 병에 걸리시지 않은 어린 원숭이들을 구하시러 돌아다니시다가 그만 병에 걸리신", { prev = true, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    if b == DIALOG_RESULT.PREV then
        goto NPC_470_0009
    end
    ::NPC_470_0011::
    b = me:dialog(npc, "원숭이들에게 왕의 증표를 빼앗기고 마셨다. 너무 흉폭하셔 내 힘으로는 되찾으실 도리가 없으시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    if b == DIALOG_RESULT.PREV then
        goto NPC_470_0010
    end
    b = me:dialog(npc, "그걸 자네가 도로 되찾아주셨으면 좋으시겠군.", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    local q8 = me:quest(quest.QUEST_SKULL_NECKLACE_8)
    if q8 == nil then
        q8 = me:start_quest(quest.QUEST_SKULL_NECKLACE_8)
        if q8 == nil then
            return true
        end
    end
    if q8 then
        q8:step(1)
    end
    return true
end

local function do_sub8_turnin(me, npc)
    if not me:has_items("두목의증표", 1) then
        me:dialog(npc, "흉폭한 원숭이들에게 왕의 증표를 되찾아주셨으면 좋으시겠다.", { prev = false, next = false })
        return true
    end
    ::NPC_470_0020::
    local b = me:dialog(npc, "오, 이런 고마우실데가 계실까! 음, 내가 자네를 믿으시길 잘하셨네! 답례를 하나 하시겠네!", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "이건 내가 요 앞에 보초를 서고있으시는 병사들의 방패를 흉내내시어 만들어보신 물건이시네.", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    local code = me:exchange(
        { ['item'] = { ["두목의증표"] = 1 } },
        { ['item'] = { ["사각방패"] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
        return true
    elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 사각방패를 줄 수 없습니다.", { prev = false, next = false })
        return true
    end
    local q8 = me:quest(quest.QUEST_SKULL_NECKLACE_8)
    if q8 then
        q8:step(2)
    end
    return true
end

function NPC_470(me, npc)
    local q8 = me:quest(quest.QUEST_SKULL_NECKLACE_8)

    if q8 == nil or q8:step() == 0 then
        if do_sub8_start(me, npc) then
            return
        end
        return
    end

    if q8:step() == 1 then
        if do_sub8_turnin(me, npc) then
            return
        end
        return
    end

    local main_q = me:quest(quest.QUEST_SKULL_NECKLACE)
    if main_q then
        local s = main_q:step()
        if s == 16 then
            ::NPC_470_0030::
            local b = me:dialog(npc, "뭐? 참원왕께서 의견을 물어보고 계신다는 것이신가?", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_470_0031::
            b = me:dialog(npc, "흠, 마침 좋은 생각을 하나 떠올리셨다. 지금 흉악해진 원숭이들로부터 우리 원숭이들도 몸을 지켜야만하시지.", { prev = true, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            if b == DIALOG_RESULT.PREV then
                goto NPC_470_0030
            end
            b = me:dialog(npc, "해서 나는 우리 원숭이들이 스스로를 지키실 수 있도록 무장을 하실 필요가 있다고 생각하신다. 인간들처럼 말이시다.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            main_q:step(17)
            return
        end
        if s == 17 then
            me:dialog(npc, "참원왕에게 내 의견을 전해주셨으면 하시다.", { prev = false, next = false })
            return
        end
        if s == 18 then
            if not me:has_items("철도", 3) or not me:has_items("사각방패", 3) then
                me:dialog(npc, "철도 3개와 사각방패 3개정도면 자경단을 꾸릴 수 있을 것 같으시다.", { prev = false, next = false })
                return
            end
            ::NPC_470_0040::
            local b = me:dialog(npc, "정말 수고하셨다. 멋진 무기들이시다. 이것을 기반으로 우리 원숭이들도 스스로를 지키실 수 있는 힘을 얻으셨다.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            b = me:dialog(npc, "걱정하지 않으셔도 좋으시다. 인간들과 싸우지 않으실 계획이시다. 자, 그러면 이 기쁜 소식을 참원왕께도 빨리 전해주셔야한다.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            if not me:rmitem("철도", 3, ITEM_DELETE_TYPE.GIVE) then
                me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                return
            end
            if not me:rmitem("사각방패", 3, ITEM_DELETE_TYPE.GIVE) then
                me:mkitem("철도", 3)
                me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                return
            end
            main_q:step(19)
            return
        end
        if s == 19 then
            me:dialog(npc, "이 기쁜 소식을 참원왕에게도 전해주셔야 한다.", { prev = false, next = false })
            return
        end
        if s == 31 then
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
            main_q:step(32)
            return
        end
    end

    me:dialog(npc, "준비중입니다.", { prev = false, next = false })
end