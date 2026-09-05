-- npc: 오단미
local enum = require('lib.enum')
local festival = require('lib.festival')
local quest = require('lib.quest')

local function seotdal_flags(q)
    local p = (q ~= nil and q:param()) or ''
    local y, w, d, b = string.match(p, '^(%d+):(%d+):(%d+):(%d+)$')
    if y ~= nil then
        return tonumber(w) or 0, tonumber(d) or 0, tonumber(b) or 0
    end
    w, d, b = string.match(p, '^(%d+):(%d+):(%d+)$')
    return tonumber(w) or 0, tonumber(d) or 0, tonumber(b) or 0
end

local function set_seotdal_flags(q, w, d, b)
    local year = festival.lunar_year()
    q:param(string.format('%d:%d:%d:%d', year, w, d, b))
end

local function run_seotdal(me, npc)
    local q = quest.get_annual(me, quest.QUEST_SEOTDAL_GIFTS)
    if q == nil or q:step() < 1 or q:completed() then
        return false
    end

    local w, d, b = seotdal_flags(q)
    if d == 1 or me:has_items('귤', 1) then
        me:dialog(npc, "이미 선릉이에게 전해 달라고 귤을 드렸답니다. 잘 전해 주세요!", { prev = false, next = false })
        return true
    end

    if not me:has_items('곶감', 1) then
        return false
    end

    local btn = me:dialog(npc, "어머, 선릉이가 선물을 보냈어요? 정말 기뻐요!", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, "감사의 뜻으로 귤을 드릴게요. 선릉이에게 전해 주세요.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local code = me:exchange(
        { ['item'] = { ['곶감'] = 1 } },
        { ['item'] = { ['귤'] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "곶감을 가지고 오세요.", { prev = false, next = false })
        return true
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 귤을 드릴 수 없습니다.", { prev = false, next = false })
        return true
    end

    set_seotdal_flags(q, w, 1, b)
    me:dialog(npc, "선릉이에게 귤 잘 전해 주세요!", { prev = false, next = true })
    return true
end

-- Dialog from yeondailyy blog screenshots (2023-11-15 post images).
-- TODO(2026-음력10월 / 김장절): 배추·총각·동치미·열무 재료표는 trash 없음. 조사일 양력 2026-08-10.
local function run_kimjang(me, npc)
    local btn = me:dialog(npc, "나연이가 제가 김장재료를 준비하기로 했다고 말했다는 말이에요?", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "아니 나연이 얘는 무슨 거짓말을 하는거야! 김장 준비는 나연이가 전부다 한다구요. 괜히 자기가 하기 싫으니까 나한테 떠넘기는것좀 봐..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    btn = me:dialog(npc, "어쩔수 없죠 뭐. 자호의가죽 1개와 여우모피 5개를 저한테 주시면 김장 재료를 준비해 드리죠. 요즘 날씨가 추워서...", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "자호의 가죽과 여우모피 다섯개를 가지고 오셨나요", {
        "네 가지고 왔어요.",
        "아니오, 없어요",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel ~= 1 then
        return
    end

    if not me:has_items("자호의가죽", 1) or not me:has_items("여우모피", 5) then
        return
    end

    local kimchi_sel, kimchi_btn = me:list(npc, "어떤 재료를 원하시나요?", {
        "배추김치를 담글래요.",
        "총각김치를 담글래요.",
        "동치미를 담글래요.",
        "파김치를 담글래요.",
        "열무김치를 담글래요.",
    }, { prev = false })
    if kimchi_btn == DIALOG_RESULT.QUIT or kimchi_sel == nil then
        return
    end

    if kimchi_sel ~= 4 then
        -- TODO(2026-음력10월 / 김장절): other kimchi material sets not transcribed / not in trash
        return
    end

    btn = me:dialog(npc, "그럼 김장재료를 드리지요...", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ["자호의가죽"] = 1, ["여우모피"] = 5 } },
        { ['item'] = { ["파"] = 1, ["고춧가루"] = 1, ["액젓"] = 1, ["마늘"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        return
    end
    if code == enum.exchange_result.LACK_CAPACITY then
        return
    end

    btn = me:dialog(npc, "어머? 소금이 없네요? 죄송해서 어쩌죠....", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc, "소금은 칠석집의 주연언니에게 달라고 하시면 줄꺼에요..", { prev = false, next = true })
end

return {
    on_click = function(me, npc)
        local btn = me:dialog(npc, "안녕하세요? 저는 오단미입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if festival.is('섣달') then
            if run_seotdal(me, npc) then
                return
            end
            me:dialog(npc, "선릉이에게 받은 곶감이 있으시면 저에게 전해 주세요.", { prev = false, next = false })
            return
        end

        if festival.is('김장') then
            run_kimjang(me, npc)
            return
        end

        if not festival.is('단오') then
            return
        end

        local sel, list_btn = me:list(npc, "제가 도와드릴 일이 있나요?", {
            "단오엔 무슨 일을 하나요?",
            "창포를 가지고 왔어요.",
            "비녀를 만들어 주세요.",
            "머리를 감고 왔어요.",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end

        if sel == 1 then
            btn = me:dialog(npc, "음력 5월 5일은 단오에요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "단오는 우리나라 3대 명절중 하나에요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "남자들은 씨름 대회를 열고, 여자들은 그네를 뛰면서 즐겁게 노는 날이죠.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "단오에 비가 오면 그 해에 풍년이 든다는 이야기가 있는데.. 이번 단오에 비가 오려나...", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "아, 참! 단오에 창포물로 머리 감는건 알고 계시죠?", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "창포를 구해오시면 제가 창포물을 만들어 드릴께요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "저희 집 마당의 우물에서 머리를 감으세요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        if sel == 2 then
            btn = me:dialog(npc, "창포를 구해오셨나요?", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "어디보자....", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local reward = nil
            local r = math.random(1, 3)
            if r == 1 then
                reward = { ['item'] = { ["창포뿌리"] = 1 } }
            elseif r == 3 then
                reward = { ['item'] = { ["창포물"] = 1 } }
            end
            local code = me:exchange(
                { ['item'] = { ["창포"] = 1 } },
                reward
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, "창포를 가지고 오세요.", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 받을 수 없어요. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
                return
            end
            if reward and reward['item']["창포뿌리"] then
                btn = me:dialog(npc, "이 창포는 뿌리가 매우 좋네요.. 창포물 만들긴 좀 아까운데요?", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "창포뿌리를 다섯개 모아오시면 제가 비녀를 만들어 드리죠.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
            elseif reward and reward['item']["창포물"] then
                btn = me:dialog(npc, "이 창포로 창포물을 만들어 드릴께요..", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "그럼 저희집 마당의 우물가에서 머리를 감으세요. 그러면 머리카락에 윤기가 생기고 빠지지 않게 된답니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
            else
                btn = me:dialog(npc, "어머, 이건 썩은 창포잖아요.. 이걸로는 창포물을 만들수가 없어요..", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
            end
            return
        end

        if sel == 3 then
            btn = me:dialog(npc, "창포뿌리를 다섯개나 모으셨어요? 그럼 제가 비녀를 만들어 드릴께요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "매달마다 받으시는 아이템을 다 모으시면 좋은일이 생길테니.. 잘 보관하세요. 단오의 아이템은 비녀에요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local code = me:exchange(
                { ['item'] = { ["창포뿌리"] = 5 } },
                { ['item'] = { ["비녀"] = 1 } }
            )
            if code == enum.exchange_result.LACK_COST then
                btn = me:dialog(npc, "창포뿌리 다섯개를 가져오시면 비녀를 만들어 드릴게요.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 비녀를 받을 수 없어요. 자리 좀 비우고 다시 오세요.", { prev = false, next = false })
                return
            end
            me:dialog(npc, "창포뿌리 다섯개 받았어요. 비녀 만들어 드렸어요.", { prev = false, next = false })
            return
        end

        if sel == 4 then
            if me:isbuff("반짝반짝") then
                if me:mkitem("망개떡", 1) == nil then
                    me:dialog(npc, "소지품이 가득 차서 망개떡을 드릴 수 없습니다.", { prev = false, next = false })
                    return
                end
                btn = me:dialog(npc, "제가 떡을 준비했는데.. 배고플때 드세요. ^^", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                me:dialog(npc, "망개떡을 드릴께요.. 맛있게 드세요~~", { prev = false, next = true })
            else
                me:dialog(npc, "밖의 우물에서 머리를 감고 오세요.", { prev = false, next = true })
            end
        end
    end
}
