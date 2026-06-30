-- npc: 부여성북문지기
local quest = require('lib.quest')
local enum = require('lib.enum')
function NPC_544(me, npc)
    local q = me:quest(quest.QUEST_JINHWANG)
    if q == nil or q:step() ~= 2 then
        return
    end

    if not me:has_items("종이쪽지", 1) then
        me:dialog(npc, "퀘스트 오류입니다.\n\n운영자에게 문의하세요.", false, true)
        return
    end

    ::NPC_544_0000::
    local button = me:dialog(npc, "오! 이 종이쪽지는 제친구로부터의 편지군요! 이친구 아직도 잘 살아있나요? 이 친구가 없었다면 전 이미 이세상에 없을겁니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_544_0001::
    local sel, list_btn = me:list(npc, "이런 고마울데가...", {
        "편지 여기있습니다.",
        "전 다른일이 있어서...",
    }, true)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto NPC_544_0000
    end
    if sel == nil or sel ~= 0 then
        return
    end

    ::NPC_544_0002::
    button = me:dialog(npc, "아 그런가요? 정말 반갑네요. 이렇게 징표라도 보게 되니. 보답으로 무엇이든 들어드리겠습니다!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_544_0003::
    button = me:dialog(npc, "천상복숭아라. 저희집에 가보로 내려오는 물건이지만 목숨보다 귀하진 않습니다. 기꺼이 드리도록 하죠. 좋은곳에 써주세요~", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_544_0002
    end

    local required = { ["종이쪽지"] = 1 }
    local code = me:exchange(
        { ['item'] = required },
        { ['item'] = { ["천상복숭아"] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        if not me:has_items("종이쪽지", 1) then
            me:dialog(npc, "퀘스트 오류입니다.\n\n운영자에게 문의하세요.", false, false)
        else
            me:dialog(npc, "종이쪽지가 없지 않은가.", false, false)
        end
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품 칸을 2칸 이상 비워주세요.", false, false)
        return
    end

    me:push_achievement(511, "진황보검을 찾아서 (흉노족장로를 찾아가자)", 7, 20)
    q:step(3)
end
