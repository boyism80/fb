-- npc: 일본전설대장장이
local quest = require('lib.quest')
local server = require('lib.server')
function NPC_451(me, npc)
    local CLASS_TO_WEAPON = {
        [CLASS.WARRIOR] = "진월신검",
        [CLASS.ROGUE]   = "청월기창",
        [CLASS.MAGE]    = "황화곤봉",
        [CLASS.POET]    = "적화접선",
    }
    local weapon_name = CLASS_TO_WEAPON[me:class()]
    if weapon_name == nil then
        local button = me:dialog(npc, "공수래 공수거... 인생이란 참 허망하지 않은가?", false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local function get_equipped_weapon_name()
        local w = me:weapon()
        if w == nil or w:model() == nil then
            return nil
        end
        return w:model():name()
    end

    local q = me:quest(quest.QUEST_JAPAN_LEGEND_WEAPON)
    local first_visit = (q == nil or q:step() == 0)

    if first_visit then
        if get_equipped_weapon_name() ~= weapon_name then
            local button = me:dialog(npc, "공수래 공수거... 인생이란 참 허망하지 않은가?", false, false)
            if button == DIALOG_RESULT.QUIT then
                return
            end
            return
        end
        local button = me:dialog(npc, "이곳에 사람이 찾아온 건 참 오래간만이군...", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "잠깐... 자네가 들고 있는 무기는 혹시... " .. weapon_name .. " 아닌가? 정말 오래간만에 보는군.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "하지만 어째 좀 조잡해 보이는군. 솜씨 좋은 장인이 만든 물건이 아니야... 그 물건의 진짜 모습은 그런 것이 아닐세.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        local sel, btn = me:list(npc, "오래간만에 그 무기를 보니 의욕이 솟는군... 어떤가, 재료만 가져다주면 그 무기의 진짜 모습을 되찾도록 해 주겠네. 물론 지금 쓰고 있는 것보다 훨씬 강해지겠지.", { "예, 잘 부탁드립니다!", "고맙지만 사양하겠습니다." }, false)
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            local b = me:dialog(npc, "그래... 생각이 바뀌면 다시 찾아오게나.", false, false)
            if b == DIALOG_RESULT.QUIT then
                return
            end
            return
        end
        button = me:dialog(npc, "좋아, 그럼 필요한 재료를 준비해오게. 재료는... 금강석 하나와 낡은망치 하나면 되네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "낡은망치라면 이 표신궁 말고 신궁에 있는 애송이가 가지고 있을 게야.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "금강석은 요즘 구하기가 어렵지만... 언젠가 한번 이 근처에서 사람을 만난 적이 있는데, 금강석을 하나 가지고 있는 것 같더군. 뭐, 조각 하나로는 아무것도 할 수 없긴 하지만...", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "겉보기엔 책상물림 같아 보이던데... 요즘은 통 바깥에 보이질 않더군. 뭐, 아무튼 수고하게.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if q == nil then
            q = me:start_quest(quest.QUEST_JAPAN_LEGEND_WEAPON)
            if q == nil then
                me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
                return
            end
        end
        if q ~= nil then
            q:step(1)
        end
        return
    end

    local button = me:dialog(npc, "왔군. 가공을 하고 싶은 무기를 손에 들고 있어야 내가 알아볼 수 있으니, 꼭 손에 들도록 하게.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    local sel, btn = me:list(npc, "자... 재료인 금강석과 낡은망치는 구해 왔는가?", { "예, 구해 왔습니다.", "아뇨, 아직..." }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 1 then
        local b = me:dialog(npc, "준비가 되면 나를 찾아 오시게나...", false, false)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if not me:has_items("금강석", 1) then
        me:dialog(npc, "금강석은 요즘 구하기가 어렵지만... 자네라면 구할 수 있을것일세.", false, false)
        return
    end
    if not me:has_items("낡은망치", 1) then
        me:dialog(npc, "낡은망치라면 이 표신궁 말고, 신궁에 있는 애송이 녀석이 가지고 있을테니 찾아가 보게나.", false, false)
        return
    end

    if get_equipped_weapon_name() ~= weapon_name then
        me:dialog(npc, "가공을 하고 싶은 무기를 손에 들고 있어야 내가 알아볼 수 있으니, 손에 들고 오도록 하게나...", false, false)
        return
    end

    button = me:dialog(npc, "음, 확실히 금강석과 낡은망치를 구해 왔군. 수고했네.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_451_0002::
    local list_sel, list_btn = me:list(npc, "가지고 있던 무기는 어떤 것이었지? 늙으니 기억력이 좋지 않군...", {
        "진월신검 입니다.",
        "청월기창 입니다.",
        "황화곤봉 입니다.",
        "적화접선 입니다.",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or list_sel == nil then
        return
    end

    local weapon_cur = weapon_name
    local weapon_new
    if list_sel == 0 then
        weapon_cur = "진월신검"
        weapon_new = "진일신검"
    elseif list_sel == 1 then
        weapon_cur = "청월기창"
        weapon_new = "청일기창"
    elseif list_sel == 2 then
        weapon_cur = "황화곤봉"
        weapon_new = "황염곤봉"
    else
        weapon_cur = "적화접선"
        weapon_new = "적염곤봉"
    end

    if get_equipped_weapon_name() ~= weapon_cur then
        me:dialog(npc, "자네... 무엇을 한건가? 뭔가... 잘못된것 같군.", false, false)
        return
    end

    ::NPC_451_0003::
    button = me:dialog(npc, "좋아! 그럼 가공을 시작하도록 하지. 잘 듣게, 가공 도중에는 절대로 허튼 수작 부리지 말게. 말도 걸지 말고. 자칫하면 재료가 모두 사라지는 수가 있어! 그 때에는 절대 책임을 지지 않으니 알아서 하게.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_451_0002
    end
    button = me:dialog(npc, "그럼 시작하겠네!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "하압!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "깡! 깡! 깡! 깡! ", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "치익~~~!!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if get_equipped_weapon_name() ~= weapon_cur then
        me:dialog(npc, "자네... 무엇을 한건가? 뭔가... 잘못된것 같군.", false, false)
        return
    end

    if not me:has_items("금강석", 1) or not me:has_items("낡은망치", 1) then
        me:dialog(npc, "금강석과 낡은망치를 다시 준비해 오게.", false, false)
        return
    end
    local unequipped = me:equipment_off(EQUIPMENT_PARTS.WEAPON)
    if unequipped == nil then
        me:dialog(npc, "인벤에 빈 자리가 없어서 무기를 벗을 수 없소. 자리를 비운 뒤 다시 오게.", false, false)
        return
    end
    local code = me:exchange(
        { ['item'] = { ["낡은망치"] = 1, ["금강석"] = 1, [unequipped] = 1 } },
        { ['item'] = { [weapon_new] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "낡은망치나 금강석, 무기를 제거할 수 없소.", false, false)
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 새 무기를 줄 수 없소.", false, false)
        return
    end

    button = me:dialog(npc, "...완성됐네. " .. weapon_cur .. "의 진짜 모습, " .. weapon_new .. "일세. 전보다 훨씬 강력한 무기가 되었지. 어떤가?", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "그럼 난 좀 쉬겠네. 오래간만에 일다운 일을 했더니 몸이 쑤시는군...", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
