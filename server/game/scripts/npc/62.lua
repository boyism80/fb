-- npc: 방물장수
local quest = require('lib.quest')
local enum = require('lib.enum')

local WATER_BOTTLE_PRICE = 150000
local VOUCHER_NAME = '물병보관증'
local BOTTLE_NAME = '물병'

function NPC_62(me, npc)
    local q = me:quest(quest.QUEST_WATER_BOTTLE)
    
    if q == nil then
        local btn = me:dialog(npc, '물병이 필요하신가요? 물병의 가격은 15만전입니다.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '물병을 구입하시겠습니까?', { '예', '아니오' }, false)
        if sel == nil or sel ~= 1 then
            return
        end
        local q = me:start_quest(quest.QUEST_WATER_BOTTLE)
        if q == nil then
            me:dialog(npc, '퀘스트를 시작할 수 없습니다.', false, false)
            return
        end
        me:dialog(npc, '물병을 빚는데는 시간이 걸립니다. 여기 보관증을 드릴테니 며칠후 오세요.', false, true)
        me:mkitem(VOUCHER_NAME, 1)
    end
    
    if q:completed() then
        me:dialog(npc, '이미 물병을 한번 받으셨군요.', false, true)
        return
    end
    
    local btn = me:dialog(npc, '물병을 찾으러 오셨군요.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    
    local code = me:exchange(
        { ['item'] = { [VOUCHER_NAME] = 1 }, ['money'] = WATER_BOTTLE_PRICE },
        { ['item'] = { [BOTTLE_NAME] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        if not me:has_items(VOUCHER_NAME, 1) then
            me:dialog(npc, '물병보관증을 가져오세요.', false, false)
        else
            me:dialog(npc, '15만전을 구해오세요.', false, false)
        end
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 물병을 받을 수 없습니다.', false, false)
        return
    end
    q:complete()
    me:dialog(npc, '여기있습니다.', false, true)
end