-- npc: 백몽연
local quest = require('lib.quest')
local enum = require('lib.enum')

local function run_material_list(me, npc)
    ::NPC_353_0008::
    local btn = me:dialog(npc, "동지집에 연이 있을텐데.. 동지집에 가서 수인이에게 연을 받아주시구요..\n\n칠석집엔 실패가 있을꺼에요.. 칠석집의 제 동생 주연이에게 실패를 받아오시구요..", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_353_0009::
    btn = me:dialog(npc, "섣달집에는 제기가 있을꺼에요.. 섣달집의 선릉이에게 제기를 받아주세요..", { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_353_0008
    end
    ::NPC_353_0010::
    btn = me:dialog(npc, "또, 김장집의 제 막내동생 나연이에게 김장독 깨진것 받아오시면 되고... \n\n추석집의 세인이에게 냄비 못 쓰는것도 받아야 하고..", { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_353_0009
    end
    ::NPC_353_0011::
    btn = me:dialog(npc, "설날집의 저희 어머니에게 널뛰기에 쓰는 널을 받아와주시고..\n\n할아버지가 계신 촌장집에 가서 종이 두장을 받아와 주세요. 아까 말씀드린 벌레 쫓는 종이에요.", { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_353_0010
    end

    ::NPC_353_0012::
    local sel, list_btn = me:list(npc, "다 아셨죠?", { "다시 알려주세요..", "네.. 받아와드릴께요." }, { prev = true })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto NPC_353_0011
    end
    if sel == 1 then
        run_material_list(me, npc)
        return
    end
    if sel ~= 2 then
        return
    end

    local q = me:start_quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil then
        return
    end
    q:step(1)
    me:dialog(npc, "그런데.. 섣달집의 선릉이를 조심하셔야 되요. 이 녀석은 발명한답시고 잡동사니들을 모으는 녀석이라..\n\n부탁드리겠습니다. 감사합니다~", { prev = false, next = true })
end

local function run_hand_in(me, npc)
    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil or q:completed() then
        return
    end

    local sel, list_btn = me:list(npc, "어떤걸 받아오셨나요?", {
        "연과 실패를 가져왔어요.",
        "제기와 깨진김장독을 가지고 왔어요.",
        "널과 찌그러진 냄비를 가지고 왔어요.",
        "벌레쫓는부적을 받아왔어요."
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        if not me:has_items("연", 1) then
            me:dialog(npc, "연은 동지집의 수인이에게 받아오시면 된답니다.", { prev = false, next = false })
            return
        end
        if not me:has_items("실패", 1) then
            me:dialog(npc, "실패는 칠석집의 주연이에게 받아오시면 된답니다.", { prev = false, next = false })
            return
        end
        if not me:rmitem("연", 1, ITEM_DELETE_TYPE.GIVE) or not me:rmitem("실패", 1, ITEM_DELETE_TYPE.GIVE) then
            return
        end
        me:dialog(npc, "아아 감사합니다~ 아, 연도 가지고 오셨는데 한번 날려보시겠어요?", { prev = false, next = true })
        me:dialog(npc, "사진도 한장 찍어드릴께요. ^^ 그럼 갑니다!!", { prev = false, next = true })
        me:holyday_screen(1, 2, {6, 8})
        return
    end

    if sel == 2 then
        if not me:has_items("제기", 1) then
            me:dialog(npc, "제기는 섣달집의 선릉이에게 받아오시면 된답니다.", { prev = false, next = false })
            return
        end
        if not me:has_items("깨진김장독", 1) then
            me:dialog(npc, "깨진김장독은 김장집의 나연이에게 받아오시면 된답니다.", { prev = false, next = false })
            return
        end
        if not me:rmitem("제기", 1, ITEM_DELETE_TYPE.GIVE) or not me:rmitem("깨진김장독", 1, ITEM_DELETE_TYPE.GIVE) then
            return
        end
        me:dialog(npc, "감사합니다~ 제기를 한번 차보세요.", { prev = false, next = true })
        me:dialog(npc, "사진 찍어드릴께요.", { prev = false, next = true })
        me:holyday_screen(0, 2, {9, 7})
        return
    end

    if sel == 3 then
        if not me:has_items("널", 1) then
            me:dialog(npc, "널은 설날집의 제 어머니에게 받아오시면 된답니다.", { prev = false, next = false })
            return
        end
        if not me:has_items("찌그러진냄비", 1) then
            me:dialog(npc, "찌그러진냄비는 추석집의 세인이에게 받아오시면 된답니다.", { prev = false, next = false })
            return
        end
        if not me:rmitem("널", 1, ITEM_DELETE_TYPE.GIVE) or not me:rmitem("찌그러진냄비", 1, ITEM_DELETE_TYPE.GIVE) then
            return
        end
        me:dialog(npc, "감사합니다!! 널뛰기 한번 해보세요! 사진도 찍어드립니다. ^^", { prev = false, next = true })
        me:holyday_screen(2, 3, {12, 5})
        return
    end

    if sel == 4 then
        local code = me:exchange(
            { ['item'] = { ["벌레쫓는부적"] = 2 } },
            { ['item'] = { ["중화절부적"] = 1, ["세시마을비서"] = 10 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, "벌레쫓는부적은 저희 할아버지가 계신 촌장집에서 받아오시면 된답니다. 두 장을 받아와 주세요.", { prev = false, next = false })
            return
        elseif code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 보상을 드리지 못합니다.", { prev = false, next = false })
            return
        end
        q:complete()
        me:dialog(npc, "다 받아오셨군요! 감사합니다!!", { prev = false, next = true })
        me:dialog(npc, "감사합니다.. 보답으로 제가 만든 부적과 비서를 드릴께요.", { prev = false, next = true })
        me:dialog(npc, "중화절부적은.. 가지고 있으면 옷이나 몸에 벼룩같은 벌레가 들지 말라고 가지고 다니는거구요, 일년동안 가지고 계시면 좋은일이 생길꺼에요.", { prev = false, next = true })
        me:dialog(npc, "세시마을비서는 어디서든 사용하면 바로 세시마을로 돌아올 수 있는 비서랍니다.", { prev = false, next = true })
        me:dialog(npc, "감사합니다! 그럼 잘 쓰시고 좋은 봄날 맞으시길!", { prev = false, next = true })
    end
end

return {
    on_click = function(me, npc)
        ::NPC_353_0001::
        local btn = me:dialog(npc, "안녕하세요? 저는 백몽연입니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        if property("sesi_rightnow") ~= 2 then
            return
        end

        local q = me:quest(quest.QUEST_BAEK_MONGYEON)

        ::NPC_353_0002::
        local sel, list_btn = me:list(npc, "안녕하세요?", {
            "중화절은 뭐하는 날인가요?",
            "도와드릴 일이라도...",
            "부탁하신일 다 했어요."
        }, { prev = true })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_353_0001
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            ::NPC_353_0003::
            btn = me:dialog(npc, "중화절은 음력 2월 1일입니다. 봄이 시작되는 때이기 때문에 창고나 부엌같은 곳을 모두 말끔히 청소하지요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_353_0004::
            btn = me:dialog(npc, "이렇게 대청소를 한 다음, 더러운 것이나 부정한 물건, 해충들을 모두 태우고 옷을 볕에 말리기도 하죠.", { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_353_0003
            end
            ::NPC_353_0005::
            btn = me:dialog(npc, "특히 초가집엔 벌레가 많아서 이것을 예방하기 위해 종이에 '향랑각시 속거천리'라고 써서 서까래에 붙여두는 풍습도 있습니다.", { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_353_0004
            end
            return
        end

        if sel == 2 then
            ::NPC_353_0006::
            btn = me:dialog(npc, "제가 중화절에 대청소하는 일을 맡아서 하고 있는데요, 마을 곳곳의 잡동사니들을 다 모아 오려니까 정말 힘이드네요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end

            if q == nil then
                ::NPC_353_0007::
                local sel2, list_btn2 = me:list(npc, "세시마을 대청소하는거.. 좀 도와주시겠어요?", { "네 도와드려야죠.", "저는 청소는 잘 못해서.." }, { prev = true })
                if list_btn2 == DIALOG_RESULT.QUIT then
                    return
                end
                if list_btn2 == DIALOG_RESULT.PREV then
                    goto NPC_353_0006
                end
                if sel2 == nil or sel2 ~= 1 then
                    if sel2 == 2 then
                        me:dialog(npc, "아... 그렇다면 어쩔 수 없죠. 혹시 마음이 바뀌면 다시 알려주세요.", { prev = false, next = false })
                    end
                    return
                end
                btn = me:dialog(npc, "감사합니다! 그럼 제가 목록을 알려드릴테니 좀 찾아와주시겠어요?", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                run_material_list(me, npc)
            else
                me:dialog(npc, "동지집에 연이 있을텐데.. 동지집에 가서 수인이에게 연을 받아주시구요..\n\n칠석집엔 실패가 있을꺼에요.. 칠석집의 제 동생 주연이에게 실패를 받아오시구요..", { prev = false, next = true })
                btn = me:dialog(npc, "섣달집에는 제기가 있을꺼에요.. 섣달집의 선릉이에게 제기를 받아주세요..", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "또, 김장집의 제 막내동생 나연이에게 김장독 깨진것 받아오시면 되고... \n\n추석집의 세인이에게 냄비 못 쓰는것도 받아야 하고..", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "설날집의 저희 어머니에게 널뛰기에 쓰는 널을 받아와주시고..\n\n할아버지가 계신 촌장집에 가서 종이 두장을 받아와 주세요. 아까 말씀드린 벌레 쫓는 종이에요.", { prev = false, next = true })
            end
            return
        end

        if sel == 3 then
            if q and q:completed() then
                me:dialog(npc, "감사합니다! 그럼 잘 쓰시고 좋은 봄날 맞으시길!", { prev = false, next = true })
                return
            end
            run_hand_in(me, npc)
            return
        end
    end
}
