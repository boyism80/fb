local ITEM_DRAGON_SCALE = '용의비늘'
local ITEM_SURYONG_SCALE = '수룡의비늘'
local ITEM_HWARYONG_SCALE = '화룡의비늘'

local function run_dragon_location_info(me, npc)
    local btn
    ::NPC_17_POS0000::
    btn = me:dialog(npc, '호랑이를 잡으려면 호랑이굴로 들어가듯 용을 잡으려면 용있는 곳으로 가야하지 않겠나!!', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_17_POS0001::
    btn = me:dialog(npc, '근데, 그 용이 사는곳이란게 애매모호 하거든?', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0000
    end
    ::NPC_17_POS0002::
    btn = me:dialog(npc, '특히 수룡과 화룡은 기타 다른 용들이 사는 곳이랑 틀리다는거야..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0001
    end
    ::NPC_17_POS0003::
    btn = me:dialog(npc, '어떤이는 극지방 구석에 있다고 하고.. 어떤이는 국내성 어딘가에 살고 있다 하고..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0002
    end
    ::NPC_17_POS0004::
    btn = me:dialog(npc, '글쎄.. 이러한 말들이 많은데.. 아직 용을 직접 본 사람이 없다네.. 물론 나도 못 봤고..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0003
    end
    ::NPC_17_POS0005::
    btn = me:dialog(npc, '우리 집안이 대대로 용에 대해서 많은 연구를 해 왔는데 말이야.. 가보처럼 전해내려오는 \'용문서\'를 보면..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0004
    end
    ::NPC_17_POS0006::
    btn = me:dialog(npc, '수룡과 화룡에 대한 이야기가 있다네.. 그곳에 보면 이 용들은 밖으로 나가길 싫어하고', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0005
    end
    ::NPC_17_POS0007::
    btn = me:dialog(npc, '아무도 만나길 원하지 않다고하지.. 그래서 호랑이의 기운을 받은자들에게 자신들을 지켜 달라고 하고 있지..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0006
    end
    ::NPC_17_POS0008::
    btn = me:dialog(npc, '만약, 자신들을 보는자가 있다면 모두 다 다시는 세상으로 못나오게 한다는 말과 함께 말일쎄..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0007
    end
    ::NPC_17_POS0009::
    btn = me:dialog(npc, '그래서 지금 그들을 호랑이들이 지키고 있다고 하더군... \'용문서\'에 나온 것은 여기까지고..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0008
    end
    ::NPC_17_POS0010::
    btn = me:dialog(npc, '이제부터는 자네가 풀어 나가야 할 걸쎄... 그리고 그 용들은 아주 난폭하다고 하니 조심하시게..', true, false)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS0009
    end
end

local function run_dragon_scale_info(me, npc)
    local btn
    ::NPC_17_POS00000::
    btn = me:dialog(npc, '안녕하신가? 그래.. 용의비늘에 대해서 궁금해서 왔다고?', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_17_POS00001::
    btn = me:dialog(npc, '허허... 용의비늘에는 여러가지가 있지.. 수룡의비늘.. 화룡의비늘.. 지룡의비늘..등등..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS00000
    end
    ::NPC_17_POS00002::
    btn = me:dialog(npc, '그 중, 옥황상제님의 병을 치료하기 위해서는 수룡,화룡의비늘이 필요하지.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS00001
    end
    ::NPC_17_POS00003::
    btn = me:dialog(npc, '하지만, 일반인들은 무엇이 수룡의비늘이고 무엇이 화룡의비늘인지 잘 구분을 못한다네..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS00002
    end
    ::NPC_17_POS00004::
    btn = me:dialog(npc, '하하하.. 이건 내 자랑이 아니고... 나는 그것을 구분할 수 있거든...', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS00003
    end
    ::NPC_17_POS00005::
    btn = me:dialog(npc, '그러니.. 용의비늘을 구하거든.. 나에게 가져 오라고.. 내가 감정을 해 줄테니..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_17_POS00004
    end

    local sel = me:list(npc, '용이 어디서 나오는지 알고 싶은가?', { '알고 싶습니다', '아니요' }, false)
    if sel == nil then
        return
    end
    if sel == 1 then
        me:dialog(npc, '그래? 그럼..자네 마음대로 하시게! 마음대로... 흥흥!!', false, false)
        return
    end
    if sel ~= 0 then
        return
    end
    run_dragon_location_info(me, npc)
end

local function run_appraise_dragon_scale(me, npc)
    local btn = me:dialog(npc, '오호.. 용의비늘을 가지고 왔다고? 어디 보세나...', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if not me:has_items(ITEM_DRAGON_SCALE, 1) then
        me:dialog(npc, '아니! 이보게.. 나한테 거짓말을 해도 되는가? ' .. name_with(ITEM_DRAGON_SCALE, '이', '가') .. ' 없지 않은가. 썩 물러 가게!!!', false, false)
        return
    end

    btn = me:dialog(npc, '오오!! 내 잠시 책을 보면서 확인하고 오겠네 잠시만 기다리게나.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local r = math.random(1, 100)
    local s = math.random(1, 100)
    me:rmitem(ITEM_DRAGON_SCALE, 1, ITEM_DELETE_TYPE.GIVE)

    if r <= 30 and s <= 30 then
        me:mkitem(ITEM_SURYONG_SCALE, 1)
        me:dialog(npc, '이것은!! 수룡의비늘이구려.. 주기 싫지만 어쩔수 없지..', false, false)
    elseif r >= 70 and s >= 70 then
        me:mkitem(ITEM_HWARYONG_SCALE, 1)
        me:dialog(npc, '이것은!! 화룡의비늘이구려.. 주기 싫지만 어쩔수 없지..', false, false)
    else
        me:dialog(npc, '에잉.. 이런 이건 그냥 비늘이 아니라 개 털뭉치가 이렇게 뭉쳐서 보인거잖나!!!', false, false)
    end
end

function NPC_17(me, npc)
    local sel = me:list(npc, '무슨일로 날 찾으시나?', {
        '용의비늘이 무엇인가요?',
        '용의비늘을 가지고 왔어요..',
    }, false)
    if sel == nil then
        return
    end
    if sel == 0 then
        run_dragon_scale_info(me, npc)
    elseif sel == 1 then
        run_appraise_dragon_scale(me, npc)
    end
end