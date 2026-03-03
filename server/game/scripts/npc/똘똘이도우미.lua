-- @note Trash: 1_초보자의길\초보자도우미.txt "똘똘이도우미". Intro dialogs + FAQ list. Account counter/exp reward omitted (no $@ var).

-- @brief   NPC Ttolttol helper: intro dialogs and FAQ list (no visit counter or exp reward).
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (똘똘이도우미).
function NPC_278(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 똘똘이 도우미 입니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, "기본적인 것들은 앞선 도우미들에게 모두 배우셨죠?", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, "저는 궁금한 것이 있다면 답변해 드리겠습니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, "열심히 공부한 학생에게는 상이 있을수도 있습니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local faq = {
        "바람의 나라의 현재 이용자보기는 어떻게 합니까?",
        "다른이에게 물건을 주려면 어떻게 합니까?",
        "멀리 떨어진 다른 사용자와 이야기는 어떻게 합니까?",
        "레벨이란 무엇입니까?",
        "힘이란 무엇입니까?",
        "지능이란 무엇입니까?",
        "민첩이란 무엇입니까?",
        "신수란 무엇입니까?",
        "직업은 무엇입니까?",
        "게시판은 무엇입니까?",
        "그만",
    }
    local answers = {
        "바람의나라 현재 이용자를 보시려면 [Ctrl]+[W]를 누르시거나, 하단 메뉴바의 네번째 버튼을 클릭하시면 보실 수 있습니다.",
        "물건을 줄 사람의 앞에서 [H]키를 눌러 원하는 물건을 건네줄 수 있습니다.",
        "귓속말을 동해 이야기를 하실 수 있으며, [Shift]키와 [']키를 동시에 누르시면 상대방의 아이디를 입력 하실수가 있습니다.",
        "바람의나라에서 레벨이란, 캐릭터의 성장 정도를 의미합니다. 레벨은 1부터 99까지 성장할 수 있으며, 레벨 5가 되면 직업을 가지셔야 그 이상 성장이 가능합니다.",
        "힘을 올리면 캐릭터의 공격력이 더 강해집니다. 또한 무거운 무기를 들 수 있게 됩니다.",
        "지능을 올리면 특정 마법의 성공 확률이 올라가게 되며, 특정 마법의 경우에는 더 강한 데미지를 갖게 됩니다.",
        "민첩을 올리면 일정 확률로 강한 공격을 하게 되며, 적으로 받게 되는 공격을 회피하는 확률이 올라가게 됩니다.",
        "바람의나라에서 신수란 각각 사방을 상징하는 '좌-청룡', '우-백호', '북-주작', '남-현무'로 구성됩니다. 신수는 게임 캐릭터를 생성할 때 선택하실 수 있고, 신수에 따라 마법 공격 이미지가 달라지게 됩니다. 신수는 한번 선택하시면 다시는 바꿀 수 없습니다.",
        "모든 유저는 레벨이 5가 되면 반드시 직업을 가져야만 성장이 가능합니다. 바람의나라에서 직업은 도적, 전사, 주술사, 도사가 존재하며 각 직업길드에서 직업을 가질 수 있습니다.",
        "바람의나라에서 게시판은 [b]키를 눌러 열 수 있으며, 하단 메뉴의 5번째 버튼을 눌러서 열 수 있습니다. 모든 게시판은 기본적으로 레벨이 5 이상부터 작성하실 수 있으며, 특정 게시판의 경우 특정 레벨에 맞는 게시판이 존재합니다.",
    }

    ::NPC_278_LIST::
    local sel, btn = me:list(npc, "그럼. 아래 중에서 궁금하신것을 선택해 주세요. 없으면 맨 아래의 [그만]을 눌러주세요.", faq, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil or sel == 10 then
        return
    end

    if sel == 7 then
        -- 신수: two dialogs in trash
        button = me:dialog(npc, "바람의나라에서 신수란 각각 사방을 상징하는 '좌-청룡', '우-백호', '북-주작', '남-현무'로 구성됩니다.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "신수는 게임 캐릭터를 생성할 때 선택하실 수 있고, 신수에 따라 마법 공격 이미지가 달라지게 됩니다.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "신수는 한번 선택하시면 다시는 바꿀 수 없습니다.", false, true)
    else
        button = me:dialog(npc, answers[sel + 1], false, true)
    end
    if button == DIALOG_RESULT.QUIT then
        return
    end
    goto NPC_278_LIST
end
