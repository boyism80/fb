local M = {}

function M.withdraw(me, npc_obj, clan)
    if clan:money() == 0 then
        me:dialog(npc_obj, '자네의 문파는 보유한 자금이 없다네.', { prev = false, next = false })
        return
    end
    local amount = me:input(npc_obj,
        string.format('그대의 문파 [%s]가 보유한 자금은 [%d]전 이라네. 제가 찾을 금액은?', clan:name(), clan:money()))
    if amount == nil then
        return
    end
    amount = math.tointeger(tonumber(amount))
    if amount == nil or amount < 0 then
        me:dialog(npc_obj, '장난이 과하군.', { prev = false, next = false })
        return
    end
    if amount == 0 then
        return
    end
    if clan:money() < amount then
        me:dialog(npc_obj, '그렇게 많이 찾을 수는 없다네.', { prev = false, next = false })
        return
    end
    local applied = me:transfer_clan_money(clan, -amount, amount)
    if applied == nil then
        me:dialog(npc_obj, '문파자금 출금 결과를 확인하지 못했습니다. 잠시 후 문파자금을 확인해 주세요.', { prev = false, next = false })
    elseif applied == false then
        me:dialog(npc_obj, '문파자금 출금에 실패했습니다.', { prev = false, next = false })
    else
        me:message(string.format('문파자금 %d전 찾았습니다.', amount), MESSAGE_TYPE.NOTIFY)
        me:message(string.format('금전 %d전 받았습니다.', amount), MESSAGE_TYPE.NOTIFY)
        me:dialog(npc_obj, string.format('자, 여기있네. 자네에게 문파자금 [%d]전을 줄테니 잘 쓰시게나.', amount),
            { prev = false, next = false })
    end
end

return M
