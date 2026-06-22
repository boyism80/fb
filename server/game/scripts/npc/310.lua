-- npc: 오엑스도우미
local npc = require('lib.npc')
local server = require('lib.server')
function NPC_310(me, npc_obj)
    ::NPC_310_000::
    local sel, btn = me:list(npc_obj, "무엇을 도와드릴까요?", {
        "물건 사기",
        "사용자 오엑스",
        "토큰 구입",
    }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.QUIT then
            return
        end
        goto NPC_310_000
    end

    if sel == 1 then
        if not me:has_items("아무나OX주최토큰", 1) then
            me:dialog(npc_obj, "아무나OX주최토큰이 없으시군요.", false, false)
            return
        end
        local sub_sel, sub_btn = me:list(npc_obj, "어떤 명령을 수행하시겠습니까?", {
            "출입구열기",
            "출입구닫기",
            "추방",
        }, false)
        if sub_btn == DIALOG_RESULT.QUIT or sub_sel == nil then
            return
        end
        if sub_sel == 0 then
            local m = me:map()
            m:block(7, 13, false)
            m:block(21, 13, false)
            me:dialog(npc_obj, "출입구를 열었습니다.", false, false)
        elseif sub_sel == 1 then
            local m = me:map()
            m:block(7, 13, true)
            m:block(21, 13, true)
            me:dialog(npc_obj, "출입구를 닫았습니다.", false, false)
        else
            local raw = me:input(npc_obj, "강제퇴장할 유저의 아이디를 적어주세요.", "퇴장할 대상의 아이디는:", "입니다.", 6, false)
            if raw == DIALOG_RESULT.QUIT then
                return
            end
            if raw == nil or raw == "" then
                me:dialog(npc_obj, "대상 아이디를 입력해 주세요.", false, false)
                return
            end
            local target = name2ch(raw)
            if target == nil then
                me:dialog(npc_obj, "해당 유저가 접속중이 아닙니다.", false, false)
                return
            end
            if me:map() ~= target:map() then
                me:dialog(npc_obj, "그럴 수 없어 뿍짝아...", false, false)
                return
            end
            local btn = me:dialog(npc_obj, raw .. "님이 강제 퇴장 됩니다..", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local exit_map = name2map("부여성")
            if exit_map then
                target:map(exit_map, 75, 140)
            end
        end
        return
    end

    if sel == 2 then
        local code = me:exchange(
            { ['item'] = { ["도깨비방망이"] = 1 } },
            { ['item'] = { ["아무나OX주최토큰"] = 1 } }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc_obj, "도깨비방망이가 없으시군요.", false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc_obj, "소지품이 가득 차서 아무나OX주최토큰을 받을 수 없습니다.", false, false)
            return
        end
        me:dialog(npc_obj, "도깨비방망이를 '아무나OX주최토큰'으로 바꿔드리겠습니다.", false, false)
    end
end
