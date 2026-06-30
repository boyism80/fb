-- npc: 북천황신수
local REQUIRED_ITEMS = { '현철', '오독산' }
local MOB_NAME = '북천황'
local SPAWN_X, SPAWN_Y = 10, 9

function NPC_147(me, npc)
    ::NPC_147_0001::
    local btn = me:dialog(npc, '이 곳은 북천황이 봉인된 곳..', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_147_0010::
    btn = me:dialog(npc, '북천황은 여러사람을 아무렇지도 않게 죽인 매우 잔인한 녀석이다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_147_0001
    end
    ::NPC_147_0011::
    btn = me:dialog(npc, '그러해서 하늘의 선녀가 북천황을 이곳에 봉인하고 나는 이 문을 지키고있다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_147_0010
    end
    ::NPC_147_0012::
    btn = me:dialog(npc, '그렇기 때문에 북천황을 부르기위해선 북천황이 다른곳으로 나갈 수 없도록 현철과 오독산으로 그문을 지켜야한다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_147_0011
    end
    ::NPC_147_0013::
    local sel, list_btn = me:list(npc, '현철과 오독산이 있는가?', { '네', '아니오' }, false)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if list_btn == DIALOG_RESULT.PREV then
        goto NPC_147_0012
    end
    if sel == nil or sel ~= 0 then
        return
    end
    local required_table = { ['현철'] = 1, ['오독산'] = 1 }
    if not me:has_items(required_table) then
        me:dialog(npc, '자네에겐 현철과 오독산이 없군.', false, false)
        return
    end
    btn = me:dialog(npc, '현철과 오독산을 가지고 있군.. 그럼 북천황을 부르겠네', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    me:spawn_mob(MOB_NAME, SPAWN_X, SPAWN_Y, false)
    for _, name in ipairs(REQUIRED_ITEMS) do
        me:rmitem(name, 1, ITEM_DELETE_TYPE.GIVE)
    end
    me:dialog(npc, '지금 북천황이 나왔네! 빠른시간내에 처리해주게!', false, true)
end
