local WATER_BOTTLE_PRICE = 150000
local VOUCHER_NAME = '물병보관증'
local BOTTLE_NAME = '물병'

function NPC_62(me, npc)
    local quest = me:quest(QUEST_WATER_BOTTLE)
    
    if quest == nil then
        local btn = me:dialog(npc, '물병이 필요하신가요? 물병의 가격은 15만전입니다.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '물병을 구입하시겠습니까?', { '예', '아니오' }, false)
        if sel == nil or sel ~= 0 then
            return
        end
        me:dialog(npc, '물병을 빚는데는 시간이 걸립니다. 여기 보관증을 드릴테니 며칠후 오세요.', false, true)
        me:mkitem(VOUCHER_NAME, 1)
        me:start_quest(QUEST_WATER_BOTTLE)
    end
    
    if quest:completed() then
        me:dialog(npc, '이미 물병을 한번 받으셨군요.', false, true)
        return
    end
    
    local btn = me:dialog(npc, '물병을 찾으러 오셨군요.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    
    if me:money() < WATER_BOTTLE_PRICE then
        me:dialog(npc, '15만전을 구해오세요.', false, false)
        return
    end
    
    me:rmitem(VOUCHER_NAME, 1, ITEM_DELETE_TYPE.GIVE)
    me:mkitem(BOTTLE_NAME, 1)
    quest:complete()
    me:dialog(npc, '여기있습니다.', false, true)
end