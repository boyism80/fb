-- npc: 흉노장로
local server = require('lib.server')
local POTION_PREFIXES = { "적", "황", "청", "녹" }

function NPC_407(me, npc)
    local button = me:dialog(npc, "비약? 누구에게 들었는지 모르지만.. 우리 흉노족의 장로에게는 비약을 만드는 법이 대대로 전해지고 있지.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "그런데, 어떤 비약을 만들려고 하는건가?", {
        "적비약을 만들고 싶습니다.",
        "황비약을 만들고 싶습니다.",
        "청비약을 만들고 싶습니다.",
        "녹비약을 만들고 싶습니다.",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 0 or sel > 3 then
        return
    end

    local prefix = POTION_PREFIXES[sel + 1]
    local button_str = prefix .. "호박단추"

    button = me:dialog(npc, prefix .. "비약을 만들기 위해선 " .. button_str .. " 2개가 필요하다네.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, "흠흠.. 그리고 말일세. 자네 요즘 중국에서 유행 한다는 '검정깃발'을 알고 있나? 그거 하나만 가졌으면 좋겠는데 말이야..", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local sel2, list_btn2 = me:list(npc, button_str .. " 2개와 검정깃발을 구해왔는가? 구해왔다면 " .. prefix .. "비약을 만들어줌세.", {
        "네, 구해왔습니다.",
        "아니오. 아직..",
    }, false)
    if list_btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
        return
    end

    if sel2 == 1 then
        me:dialog(npc, "비약을 만들고 싶으면 어서 구해오게.", false, false)
        return
    end

    local reward_name = prefix .. "비약(소)"
    local code = me:exchange(
        { ['item'] = { [button_str] = 2, ["검정깃발"] = 1 } },
        { ['item'] = { [reward_name] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        if not me:has_items(button_str, 2) then
            me:dialog(npc, "아니 자네," .. button_str .. " 2개가 없지 않은가.", false, false)
        elseif not me:has_items("검정깃발", 1) then
            me:dialog(npc, "아니 자네, 검정깃발이 없지 않은가.", false, false)
        else
            me:dialog(npc, "아니 자네," .. button_str .. " 2개와 검정깃발이 없지 않은가.", false, false)
        end
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 줄 수 없네.", false, false)
        return
    end

    me:dialog(npc, "자. 여기 " .. reward_name .. " 일세. 받게나.", false, false)
end
