-- npc: 촌장부인
local quest = require('lib.quest')
local enum = require('lib.enum')
local festival = require('lib.festival')
local function run_junghwa(me, npc)
    if not festival.is('중화절') then
        return false
    end
    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:step() < 1 then
        return false
    end
    if me:has_items("벌레쫓는부적", 2) then
        return false
    end
    local btn = me:dialog(npc, "벌써 중화절인가요? 그럼 몽연이에게 벌레 쫓는 부적을 줘야 겠네요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "이 부적은 '향랑각시 속거천리(香娘閣氏 速去千里)'라고 써있답니다. 냄새나는 벌레를 향랑각시라고 했고, 빨리 천리를 가라는 뜻이죠", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "그럼 몽연이에게 잘 전해주세요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("벌레쫓는부적", 1)
    return true
end

local function run_seokka(me, npc)
    if not festival.is('석가탄신일') then
        return false
    end
    if me:has_items("색비단", 1) then
        me:dialog(npc, "색비단으로 예쁜 연등을 만들어 잘 쓰시길 바라겠습니다.", { prev = false, next = true })
        return true
    end
    local btn = me:dialog(npc, "주지스님이 색비단을 받아오라고 하셨군요. 드려야죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "그럼 예쁜 연등 얻으셔서 잘 쓰시길 바라겠습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("색비단", 1)
    return true
end

local function run_dano(me, npc)
    if not festival.is('단오') then
        return false
    end
    local btn = me:dialog(npc, "창포가 필요하시다구요.. 제가 모아놓은 창포를 드리도록 하죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "그냥은 못드리구요.. 하나에 100원이에요. ^^", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    local sel, list_btn = me:list(npc, "창포를 사시겠어요?", { "네. 살래요", "아니오.." }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel == 2 then
        me:dialog(npc, "흥.. 시름 말구..", { prev = false, next = true })
        return true
    end
    if sel ~= 1 then
        return true
    end
    local code = me:exchange(
        { ['money'] = 100 },
        { ['item'] = { ["창포"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "창포는 하나에 100원이에요.", { prev = false, next = true })
        return true
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 창포를 줄 수 없습니다.", { prev = false, next = false })
        return true
    end
    me:dialog(npc, "창포를 드렸습니다.", { prev = false, next = true })
    return true
end

local function run_yudu(me, npc)
    if not festival.is('유두') then
        return false
    end

    local btn = me:dialog(npc, "고사를 지내시려구요? 고사를 지내려면 유두벼, 유두콩, 유두조가 필요할텐데...", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "벼, 콩, 조를 구해오시면 제가 유두벼, 유두콩, 유두조로 바꿔드릴게요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local sel, list_btn = me:list(npc, "지금 구해오셨나요?", {
        "네, 다 구해왔어요. 지금 바꿔주세요.",
        "아니오, 아직 다 못구해왔어요.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel ~= 1 then
        return true
    end

    local code = me:exchange(
        { ['item'] = { ["벼"] = 1, ["콩"] = 1, ["조"] = 1 } },
        { ['item'] = { ["유두벼"] = 1, ["유두콩"] = 1, ["유두조"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "벼, 콩, 조가 부족합니다. 다 구해오신 뒤 다시 찾아와 주세요.", { prev = false, next = false })
        return true
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 바꿔드릴 수 없습니다.", { prev = false, next = false })
        return true
    end
    me:dialog(npc, "다 됐습니다~ 그럼 경건한 마음으로 고사를 지내세요~", { prev = false, next = false })
    return true
end

local function run_chilseok(me, npc)
    if not festival.is('칠석') then
        return false
    end
    local sel, list_btn = me:list(npc, "무슨일로 오셨나요?", { "그물이 필요해요..", "식용호박이 필요해요..." }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel == nil then
        return true
    end
    if sel == 1 then
        local btn = me:dialog(npc, "그물이 필요하시다구요.. 제가 모아놓은 그물을 드리도록 하죠.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return true
        end
        btn = me:dialog(npc, "그물이 여기 있습니다. 그럼 잘 사용하세요..", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return true
        end
        me:mkitem("그물", 1)
        return true
    end
    local btn = me:dialog(npc, "식용호박이 필요하시다구요.. 제가 모아놓은 식용호박을 드리도록 하죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "식용호박이 여기 있습니다. 그럼 잘 사용하세요..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("식용호박", 1)
    return true
end

local function run_chuseok(me, npc)
    if not festival.is('추석') then
        return false
    end
    local q = me:quest(quest.QUEST_BAEKRIHYANG)
    local sel, list_btn = me:list(npc, "무슨일로 오셨나요?", { "벌초를 하고 왔어요", "식용호박이 없어서..." }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return true
    end
    if sel == nil then
        return true
    end
    if sel == 1 then
        local btn = me:dialog(npc, "벌초를 하고 오셨어요?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return true
        end
        if q == nil or q:step() < 1 or q:progress() < 1 then
            me:dialog(npc, "벌초를 하지 않으셨네요? 절 속이시면 안돼죠..", { prev = false, next = false })
            return true
        end
        me:dialog(npc, "수고하셨어요. 올게심니를 드릴께요.. 매달마다 특정 아이템을 하나씩 모아서 1년동안 모으시면 좋을일이 생길꺼에요. 추석엔 올게심니에요.", { prev = false, next = false })
        if me:mkitem("올게심니", 1) == nil then
            me:dialog(npc, "소지품이 가득 차서 올게심니를 줄 수 없습니다.", { prev = false, next = false })
            return true
        end
        q:progress(0)
        return true
    end
    local btn = me:dialog(npc, "식용호박이 필요하시다구요.. 제가 가지고 있는 식용호박을 드리도록 하죠.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:mkitem("식용호박", 1)
    me:dialog(npc, "식용호박이 여기 있습니다. 그럼 잘 사용하세요..", { prev = false, next = false })
    return true
end

return {
    on_click = function(me, npc)
        local btn = me:dialog(npc, "저는 이 마을 촌장의 부인되는 목원지라고 합니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if run_junghwa(me, npc) then
            return
        end
        if run_seokka(me, npc) then
            return
        end
        if run_dano(me, npc) then
            return
        end
        if run_yudu(me, npc) then
            return
        end
        if run_chilseok(me, npc) then
            return
        end
        if run_chuseok(me, npc) then
            return
        end
    end
}
