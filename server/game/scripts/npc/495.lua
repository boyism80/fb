-- npc: 출발도우미
local server = require('lib.server')
local MAP_ID_LOBBY = 12010
local MAP_ID_GAME = 12011
local MAP_ID_FINISH = 12016
local MAP_ID_DEATH = 12017

local SURVIVE_ITEMS = { ["노란비서"] = 1, ["소환비서"] = 1, ["동동주"] = 1, ["막걸리"] = 1 }

function NPC_495(me, npc)
    local m = me:map()
    if m == nil or m:model() == nil then
        return
    end
    local map_id = m:model():id()

    if map_id == MAP_ID_DEATH then
        local btn = me:dialog(npc, "아이고, 저런 죽으셨군요!", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "살려드릴테니 어서 다시 달리기에 참여하러 가보세요.", false, false)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if me:state() == STATE.GHOST then
            me:state(STATE.NORMAL)
            me:hp(me:maxhp())
        end
        local lobby = name2map("서바이벌대기실")
        if lobby then
            local x = math.random(45, 55)
            local y = math.random(44, 54)
            me:map(lobby, x, y)
        end
        return
    end

    if map_id == MAP_ID_FINISH then
        local rank = property("surviverun_rank")
        if rank == nil or rank == 0 then
            rank = 1
        end
        local prize_name = "노란비서"
        if rank == 1 then
            prize_name = property("survive_prize_1") or prize_name
        elseif rank == 2 then
            prize_name = property("survive_prize_2") or prize_name
        elseif rank <= 10 then
            prize_name = property("survive_prize_10") or prize_name
        else
            prize_name = property("survive_prize_joiner") or prize_name
        end
        local btn = me:dialog(npc, "축하합니다! " .. me:name() .. "님은 " .. tostring(rank) .. "등으로 완주하여 상품으로 " .. prize_name .. "을(를) 받으실 수 있습니다.", false, false)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if me:mkitem(prize_name, 1) ~= nil then
            property("surviverun_rank", rank + 1)
        end
        local exit_map = name2map("부여성")
        if exit_map then
            me:map(exit_map, 70, 140)
        end
        return
    end

    if map_id == MAP_ID_LOBBY then
        local game_start = property("survive_game_start")
        local game_end = property("survive_game_end")
        local now_ok = (game_start == nil and game_end == nil) or true

        if not me:has_items("서바이벌증표", 1) then
            local sel, list_btn = me:list(npc, "[서바이벌증표]가 없으시군요. 이벤트에 참여하시려면, 서바이벌 증표가 필요해요.", {
                "네. 하나만 만들어 주세요.",
                "서바이벌증표는 어떻게 얻나요?",
            }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 then
                local code = me:exchange(
                    { ['item'] = SURVIVE_ITEMS },
                    { ['item'] = { ["서바이벌증표"] = 1 } }
                )
                if code == server.EXCHANGE_RESULT.LACK_COST then
                    local lack = {}
                    if not me:has_items("노란비서", 1) then lack[#lack + 1] = "노란비서" end
                    if not me:has_items("소환비서", 1) then lack[#lack + 1] = "소환비서" end
                    if not me:has_items("동동주", 1) then lack[#lack + 1] = "동동주" end
                    if not me:has_items("막걸리", 1) then lack[#lack + 1] = "막걸리" end
                    if #lack > 0 then
                        me:dialog(npc, "저런, " .. me:name() .. "님은 " .. table.concat(lack, ", ") .. " 가 부족하시군요. 어서 구해오세요.", false, false)
                    else
                        me:dialog(npc, "아이템을 건네지 못했습니다.", false, false)
                    end
                    return
                elseif code == server.EXCHANGE_RESULT.LACK_CAPACITY then
                    me:dialog(npc, "소지품이 가득 차서 증표를 줄 수 없습니다.", false, false)
                    return
                end
                me:dialog(npc, "서바이벌증표를 드렸습니다. 중요한 물건이니 절대 잊어버리지 않도록 주의 해 주세요.", false, false)
            else
                me:dialog(npc, "서바이벌증표는 [노란비서], [소환비서], [동동주], [막걸리]를 가지고 오시면 만들어 드리고 있습니다.", false, true)
                me:dialog(npc, "만약 없으시다면 북서, 북동, 남서, 남동쪽의 npc가 하나씩 나누어드리고 있으니 가서 받아오세요.", false, false)
            end
            return
        end

        if not now_ok then
            me:dialog(npc, "지금은 이벤트 진행시간이 아닙니다. ", false, true)
            local sel, list_btn = me:list(npc, "퇴장 하시겠습니까?", { "네. 퇴장합니다.", "아니오. 이따가요." }, false)
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 0 then
                local exit_map = name2map("부여성")
                if exit_map then
                    me:map(exit_map, 70, 140)
                end
            end
            return
        end

        local sel, list_btn = me:list(npc, "[서바이벌증표]를 가지고 있으시군요. 서바이벌 달리기에 참여하시겠습니까?", {
            "네. 참여합니다.",
            "아니오. 이따가요.",
            "퇴장합니다.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            me:dialog(npc, "[서바이벌달리기] 에서는 몬스터에게 단 한대만 맞더라도, 바로 사망을 하게 됩니다.\n\n하지만 죽음에 따른 패널티는 따로 없으니 너무 걱정은 하지 않으셔도 됩니다.\n그럼 입장시켜 드리겠습니다.", false, false)
            local game_map = name2map("서바이벌달리기")
            if game_map then
                local x = math.random(6, 13)
                local y = 5 + math.random(0, 3)
                me:map(game_map, x, y)
            end
        elseif sel == 2 then
            me:dialog(npc, "퇴장하시겠다구요? [서바이벌증표]를 회수하고, 퇴장을 도와드리겠습니다.", false, true)
            local sel2, list_btn2 = me:list(npc, "서바이벌증표를 회수합니다.", { "네. 퇴장합니다.", "아니오. 이따가요." }, false)
            if list_btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
                return
            end
            if sel2 == 0 then
                if me:rmitem("서바이벌증표", 1, ITEM_DELETE_TYPE.GIVE) then
                    local exit_map = name2map("부여성")
                    if exit_map then
                        me:map(exit_map, 70, 140)
                    end
                end
            end
        end
        return
    end

    local join_start = property("survive_join_start")
    local join_end = property("survive_join_end")
    local in_join = (join_start == nil and join_end == nil) or true

    if not in_join then
        me:dialog(npc, "현재는 참여 가능한 시간이 아닙니다. 자세한 내용은 공지사항(이벤트)을 참고해 주세요.", false, false)
        return
    end

    local btn = me:dialog(npc, "안녕하세요? 서바이벌 달리기는 곳곳에 위치한 함정과 몬스터를 피해 결승 지점까지 골인을 목표로 달리는 이벤트 게임 입니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "이벤트의 승리 조건은, 가장 먼저 입장하신 분부터 순차적으로 준비된 상품을 제공해 드리는 게임 입니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local sel, list_btn = me:list(npc, "서바이벌 달리기에 입장하시겠습니까?", {
        "네. 입장합니다.",
        "서바이벌 달리기는 무엇인가요?",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        local lobby = name2map("서바이벌대기실")
        if lobby then
            local x = math.random(45, 55)
            local y = math.random(44, 54)
            me:map(lobby, x, y)
        else
            me:dialog(npc, "입장할 수 없습니다.", false, false)
        end
    else
        me:dialog(npc, "안녕하세요? 저는 서바이벌 달리기가 무엇인지 설명해드릴 출발도우미 입니다.", false, true)
        me:dialog(npc, "서바이벌달리기는 골인지점까지 살아남아 빠르게 도착한 순서대로 상품을 드리는 이벤트 입니다.", false, true)
        me:dialog(npc, "서바이벌달리기 내부에서 몬스터에게 죽는다고 하더라도, 별도의 패널티는 존재하지 않으니 안심하셔도 됩니다.", false, true)
        me:dialog(npc, "서바이벌달리기 이벤트에 참여하시기 위해서는 [서바이벌증표]라는 아이템이 필요한데, 이 것은 동동주, 막걸리, 소환비서, 노란비서가 있어야 만드실 수 있으니 미리 준비해 가시는 것도 하나의 방법입니다.", false, true)
        me:dialog(npc, "모든 준비가 되시면 다시 저를 찾아주세요.", false, false)
    end
end
