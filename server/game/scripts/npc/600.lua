-- npc: 북극상인
local enum = require('lib.enum')
local VOUCHER_BASE_PRICE = 500000
local VOUCHER_PRICE_PER_WEEK = 10000
local REF_TIMESTAMP = 1510704000
local SEC_PER_WEEK = 604800

local function voucher_price()
    local now_ts = now()
    if not now_ts or now_ts <= 0 then
        return VOUCHER_BASE_PRICE
    end
    local weeks = math.floor((now_ts - REF_TIMESTAMP) / SEC_PER_WEEK)
    if weeks < 0 then
        weeks = 0
    end
    return VOUCHER_BASE_PRICE + (weeks * VOUCHER_PRICE_PER_WEEK)
end

function NPC_600(me, npc)
::MENU::
    local sel, btn = me:list(npc, "안녕하신가? 그래.. 요즘 하는 일은 잘 되고? 내게 무슨 볼일로 찾아왔나? 설마 인사차 온건 아닐테고.. 허허허 ~", {
        "비패교환증을 사러 왔어요 !!",
        "문파비패를 사러 왔어요 !!",
        "인사차 왔어요."
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        local price = voucher_price()
        local d = me:dialog(npc, string.format("음 .. 비패교환증을 구입하러 왔단 말이지. 비패교환증을 사기 위해서는 %d전이 필요하다네. 서버가 시작된 이래로 계속해서 값이 증가하지.", price), { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        local sel2, btn2 = me:list(npc, "자네에게 그만한 돈이 있나?", {
            "예, 돈을 준비해 왔어요.",
            "아니오, 돈을 준비해서 다음에 올께요."
        }, { prev = false })
        if btn2 == DIALOG_RESULT.QUIT then
            return
        end
        if sel2 == 2 then
            me:dialog(npc, "그래, 잘 가게나~", { prev = false, next = false })
            return
        end
        if sel2 ~= 1 then
            goto MENU
        end
        local code = me:exchange(
            { ['money'] = price },
            { ['item'] = { ["비패교환증"] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, string.format("아니! 누굴 놀리나!! %d전이 없지 않나!! 장난하지 말게!!", price), { prev = false, next = false })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 비패교환증을 줄 수 없습니다.", { prev = false, next = false })
            return
        end
        goto MENU
    end

    if sel == 2 then
        local d = me:dialog(npc, "음 .. 문파비패를 구입하러 왔단 말이지. 문파비패를 사기 위해서는 비패교환증이 필요한데...", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        d = me:dialog(npc, "좋아, 문파를 만들기 위해서는 5개의 문파비패를 모아야 한다는건 알고 있겠지?", { prev = false, next = true })
        if d == DIALOG_RESULT.QUIT then
            return
        end
        local sel2, btn2 = me:list(npc, "아래의 5개 중 어느 것을 원하나?", {
            "[파랑문파비패]를 주세요.",
            "[노랑문파비패]를 주세요.",
            "[초록문파비패]를 주세요.",
            "[보라문파비패]를 주세요.",
            "[연두문파비패]를 주세요."
        }, { prev = true })
        if btn2 == DIALOG_RESULT.QUIT then
            return
        end
        if btn2 == DIALOG_RESULT.PREV then
            goto MENU
        end
        if sel2 == nil or sel2 < 1 or sel2 > 5 then
            goto MENU
        end
        local names = { "파랑문파비패", "노랑문파비패", "초록문파비패", "보라문파비패", "연두문파비패" }
        local name = names[sel2]
        local code = me:exchange(
            { ['item'] = { ["비패교환증"] = 1 } },
            { ['item'] = { [name] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "비패교환증을 제출할 수 없습니다.", { prev = false, next = false })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 문파비패를 줄 수 없습니다.", { prev = false, next = false })
            return
        end
        goto MENU
    end

    me:dialog(npc, "허허~ 싱겁기는. 나는 잘 지내고 있다네. 자네도 잘 지내시게나~", { prev = false, next = true })
    goto MENU
end