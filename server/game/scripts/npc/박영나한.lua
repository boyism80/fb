-- @note Trash: 1_중국\소림사.txt "박영나한". Re-learn 백열장/선풍각 after "forgetting": 4 intro dialogs, list choice; 백열장 100000 zeny + $palmattack$ 초기화 (rmspell); 선풍각 200000 zeny + $feetattack$ 초기화 (rmspell). FB: me:money(), me:rmspell(name).

local BAEKYEOL_PRICE = 100000
local SEONPUNG_PRICE = 200000

--- @brief NPC 박영나한: Re-learn 백열장 or 선풍각 for zeny; removes spell then player goes to 혜천대사.
--- @param me   character talking to the NPC
--- @param npc  NPC entity (박영나한)
function NPC_384(me, npc)
    local btn = me:dialog(npc, "뭐라고오옷?! 이미 배웠던 선풍각과 백열장 사용 방법을 잊어버렸다고?!", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "우리 소림사에서 알려준 비기를 그렇게 함부로 대하다니... 흥!! 자네는 어찌 된 것이 그 모양인가? ", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "그래... 어디 반성하고 있길 바라겠네. 그런데...뭐라고? 다시 배우고 싶다니 지금 나와 장난하자는 겐가? 혜천대사님이 화가날 모습을 생각하니 으으...", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "알았네. 그럼 내 혜천대사님께 잘 일러둘테니, 자네의 그 무지함을 반성하는 의미에서 소림사에 시주를 받아야겠네.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "무슨 마법을 다시 배우고 싶은겐가?", { "백열장을...", "선풍각을..." }, true)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        btn = me:dialog(npc, "그래. 백열장을 다시 배우고 싶으면 그 댓가로 10만전을 시주해야 한다네. 그리고 자네가 수련했던 백열장의 모든 기억은 사라진다네.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel2, list_btn2 = me:list(npc, "다시 수련하겠는가?", { "예. 부탁드립니다.", "아니오... 죄송합니다." }, true)
        if list_btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
            return
        end
        if sel2 == 0 then
            if me:money() < BAEKYEOL_PRICE then
                me:dialog(npc, "뭔가? 자네는 시주할 금액이 없는 것 같은데?", false, false)
                return
            end
            me:money(me:money() - BAEKYEOL_PRICE)
            me:rmspell("백열장")
            btn = me:dialog(npc, "자... 그럼 자네의 백열장에 관한 기억을 지우겠네. 마하라반야... 움메움메...", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "다 된 것 같군. 이제 혜천대사님을 찾아가게.", false, false)
        end
        return
    end

    if sel == 1 then
        btn = me:dialog(npc, "그래. 선풍각을 다시 배우고 싶으면 그 댓가로 20만전을 시주해야 한다네. 그리고 자네가 수련했던 선풍각의 모든 기억은 사라진다네.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel2, list_btn2 = me:list(npc, "다시 수련하겠는가?", { "예. 부탁드립니다.", "아니오... 죄송합니다." }, true)
        if list_btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
            return
        end
        if sel2 == 0 then
            if me:money() < SEONPUNG_PRICE then
                me:dialog(npc, "뭔가? 자네는 시주할 금액이 없는 것 같은데?", false, false)
                return
            end
            me:money(me:money() - SEONPUNG_PRICE)
            me:rmspell("선풍각")
            btn = me:dialog(npc, "자... 그럼 자네의 선풍각에 관한 기억을 지우겠네. 마하라반야... 움메움메...", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "다 된 것 같군. 이제 혜천대사님을 찾아가게.", false, false)
        end
        return
    end
end
