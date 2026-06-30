-- npc: 백룡장
local function run_pardon_placeholder(me, npc)
    me:dialog(npc, "사면 기능은 준비중입니다.", false, false)
end

local function run_nation_change(me, npc)
    local nation = me:nation()
    local target_name, target_nation, cost
    if nation == NATION.GOGURYEO then
        target_name = "부여"
        target_nation = NATION.BUYEO
        cost = 300000
    elseif nation == NATION.BUYEO then
        target_name = "고구려"
        target_nation = NATION.GOGURYEO
        cost = 30000
    else
        me:dialog(npc, "잘 가게...", false, false)
        return true
    end
    local cost_msg = (cost == 300000) and "삼십만전" or "삼만전"
    ::NPC_274_0001::
    local btn = me:dialog(npc, "내 비록 지금은 귀양을 온 처지이지만, 그 정도는 할 수 있다네... 어디보자...", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    ::NPC_274_0002::
    local sel4, lb4 = me:list(npc, "상제께 금 " .. cost_msg .. "을 제물로 바치면, " .. target_name .. " 국적을 얻도록 해주겠네...", {
        target_name .. " 국적을 얻게 해주십시요.",
        "에고, 돈이 모자라는군요, 다음에 다시...",
    }, true)
    if lb4 == DIALOG_RESULT.QUIT or sel4 == nil then
        return true
    end
    if lb4 == DIALOG_RESULT.PREV then
        goto NPC_274_0001
    end
    if sel4 == 1 then
        me:dialog(npc, "잘 가게...", false, false)
        return true
    end
    if me:money() < cost then
        me:dialog(npc, "저런, 재물이 모자라는군, 다음에 다시 오게나... 그럼...", false, false)
        return true
    end
    me:money(me:money() - cost)
    me:nation(target_nation)
    btn = me:dialog(npc, "그대의 노고를 높이 사 상제께서 그대의 소원을 들어주셨네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    me:dialog(npc, "이제 " .. target_name .. "인 답게 살아가게나.", false, false)
    return true
end

local function run_weapon_three_dialogs(me, npc, line1, line2, line3)
    local btn = me:dialog(npc, line1, false, true)
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    btn = me:dialog(npc, line2, true, true)
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if btn == DIALOG_RESULT.PREV then
        return DIALOG_RESULT.PREV
    end
    me:dialog(npc, line3, false, false)
    return true
end

local function run_weapon_info_loop(me, npc)
    ::NPC_274_0010::
    local btn = me:dialog(npc, "나는 이 세상에 존재하는 모든 무기에 대해 알고 있네. 또한 몇몇 무기는 만들 수도 있다네... 재료만 있다면...", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    while true do
        ::NPC_274_0011::
        local weapon_sel, weapon_lb = me:list(npc, "알고 싶은 것이 있는가?", {
            "진성검에 대해 알고 싶습니다.",
            "백진도에 대해 알고 싶습니다.",
            "용설봉에 대해 알고 싶습니다.",
            "진선장에 대해 알고 싶습니다.",
            "혹시 백화검을....",
            "혹시 빙룡검을....",
            "혹시 방천화극을....",
            "이만 물러가보겠습니다...",
        }, true)
        if weapon_lb == DIALOG_RESULT.QUIT or weapon_sel == nil then
            return
        end
        if weapon_lb == DIALOG_RESULT.PREV then
            goto NPC_274_0010
        end
        if weapon_sel == 7 then
            me:dialog(npc, "잘 가게나...", false, false)
            return
        end

        if weapon_sel == 0 then
            local r = run_weapon_three_dialogs(me, npc,
                "진성검을 북방대초원 지역에서 나오는 물건들로 만들어진다네. 비록 만들기는 어렵지만 엄청난 힘을 가지고 있는 검이지...",
                "그 푸르게 빛나는 검신엔 주작의 영혼이 봉해져 있다고 전해지네... 누구든지 일단 진성검을 갖게 되면 엄청난 힘을 얻을 수 있을걸세...",
                "그대가 만약 강해지고 싶다면 한번 진성검을 만들어보게나...")
            if r == false then
                return
            end
        elseif weapon_sel == 1 then
            local r = run_weapon_three_dialogs(me, npc,
                "백진도는 진성검과 마찬가지로 북방대초원 지역에서 나오는 물건들로 만들어진다네.. 이 칼도 만들기는 어렵지만 엄청난 힘을 가지고 있지..",
                "백진도의 도신은 백호의 기운으로 만들어져 있네. 백진도를 들고 있으면 피어오르는 백호의 기운을 느낄 수 있다네...",
                "그대가 만약 강해지고 싶다면 한번 백진도를 만들어보게나...")
            if r == false then
                return
            end
        elseif weapon_sel == 2 then
            local r = run_weapon_three_dialogs(me, npc,
                "용설봉도 북방대초원 지역에서 나오는 물건들로 만들어지지.. 이 봉을 들게 되면 신성한 기운이 느껴진다네...",
                "따라서 용설봉에는 청룡의 지혜가 담겨있네. 최강의 공격 주술을 구사하기 위해서는 반드시 용설봉의 도움이 필요할것이야...",
                "그대가 만약 가장 강한 주술사가 되고 싶다면 한번 만들어보게나...")
            if r == false then
                return
            end
        elseif weapon_sel == 3 then
            local r = run_weapon_three_dialogs(me, npc,
                "진선장도 북방대초원 지역에서 나오는 물건들로 만들어진다네... 진선장을 들면 생명력이 느껴진다네..",
                "진선장에는 현무의 기운이 담겨 있어서, 진선장을 들고 있으면 현무의 강력한 생명력을 느낄 수 있다고 전해지네...",
                "그대가 만약 가장 강한 도사가 되고 싶다면 한번 만들어보게나...")
            if r == false then
                return
            end
        elseif weapon_sel == 4 then
            me:dialog(npc, "백화검이라... 백화검을 만드는 데에는 세가지 재료가 필요하다네... 하지만 지금은 때가 아니야... 미안하네...", false, false)
        elseif weapon_sel == 5 then
            me:dialog(npc, "빙룡검은 백화검과 정반대의 속성을 지닌 검일세... 하지만 역시 지금은 때가 아닐세... 미안하네...", false, false)
        else
            local r = run_weapon_three_dialogs(me, npc,
                "방천화극을 구하는 방법을 알고싶다고... 음... 방천화극을 만들 줄 아는 대장장이가 있다는 소문은 들었네만...",
                "그 대장장이는 아마도 술을 무척이나 좋아한다고 들었네...",
                "자네가 귀한 술을 가지고, 그 대장장이를 찾아가기만 한다면, 아마도 만들어 줄 지도 모르지...")
            if r == false then
                return
            end
        end
    end
end

function NPC_274(me, npc)
    local sel, list_btn = me:list(npc, "zzzzzz", {
        "여보세요?",
        "......",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        me:dialog(npc, "zzzzzzzz", false, false)
        return
    end

    if sel ~= 1 then
        return
    end

    local sel2, lb2 = me:list(npc, "나는 백룡장, 나에게 볼 일이 있는가?", {
        "백룡장님은...",
        "안녕히 계세요...",
    }, false)
    if lb2 == DIALOG_RESULT.QUIT or sel2 == nil then
        return
    end

    if sel2 == 1 then
        me:dialog(npc, "잘 가게...", false, false)
        return
    end

    ::NPC_274_0003::
    local btn = me:dialog(npc, "나는 상제를 모시는 금천군 12대를 지휘했던 천계 제일의 장군 백룡장일세.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_274_0004::
    btn = me:dialog(npc, "비록 지금은 모함을 받고 여기 내려와 있네만 언젠간 반드시 상제께서 나의 결백을 알아주실 날이 올것이야...", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_274_0003
    end

    ::NPC_274_0005::
    local sel3, lb3 = me:list(npc, "알고 싶은 것이 있는가?", {
        "천계의 장군이시라면...",
        "저도 억울한데 혹시 사면을?",
        "혹시 제 국적을...",
        "네... 안녕히 계세요.",
    }, true)
    if lb3 == DIALOG_RESULT.QUIT or sel3 == nil then
        return
    end
    if lb3 == DIALOG_RESULT.PREV then
        goto NPC_274_0004
    end

    if sel3 == 3 then
        me:dialog(npc, "잘 가게...", false, false)
        return
    end

    if sel3 == 1 then
        run_pardon_placeholder(me, npc)
        return
    end

    if sel3 == 2 then
        run_nation_change(me, npc)
        return
    end

    run_weapon_info_loop(me, npc)
end
