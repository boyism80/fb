-- npc: 천성랑
local quest = require('lib.quest')
local enum = require('lib.enum')
local PROMOTION_3RD_STATS = {
    [CLASS.WARRIOR] = { min_hp = 600000,  min_mp = 0 },
    [CLASS.ROGUE]   = { min_hp = 600000,  min_mp = 0 },
    [CLASS.MAGE]    = { min_hp = 300000,  min_mp = 500000 },
    [CLASS.POET]    = { min_hp = 250000,  min_mp = 250000 },
}

local PROMOTION_4TH_WEAPON = {
    [CLASS.WARRIOR] = '검성기검',
    [CLASS.ROGUE]   = '태성태도',
    [CLASS.MAGE]    = '현자금봉',
    [CLASS.POET]    = '진선역봉',
}

local WEAPON_REISSUE_GOLD = 10000000

local function meets_4th_promotion_quest_requirements(me)
    if me:promotion() < 2 then
        return false, '3차 승급을 한 뒤에 다시 오세요.'
    end
    if me:level() < 99 then
        return false, '좀더 수련을 쌓고오세요.'
    end
    local stats = PROMOTION_3RD_STATS[me:class()]
    if not stats then
        return false, '아직 준비가 안 되었습니다.'
    end
    if me:base_hp() < stats.min_hp or me:base_mp() < stats.min_mp then
        return false, '좀더 수련을 쌓고오세요.'
    end
    return true
end

local function has_4th_weapon(me)
    local weapon_name = PROMOTION_4TH_WEAPON[me:class()]
    if not weapon_name then
        return false
    end
    local w = me:weapon()
    if w and w:model() and w:model():name() == weapon_name then
        return true
    end
    return me:has_items(weapon_name, 1)
end

function NPC_96(me, npc)
    local selected = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {
        '4차 승급을 원합니다',
        '4차 승급 무기들 다시 받고 싶습니다',
    })
    if selected == nil then
        return
    end

    if selected == 0 then
        local q = me:quest(quest.QUEST_MUTA)
        if me:has_items('반고의심장', 1) and q and not q:completed() and q:step() >= 7 then
            local choice = me:list(npc, '당신은 지금껏 수많은 수련을 거쳐 인간으로서 오르기 힘든 경지에 도달하셨고, 더욱이 세상을 종말로부터 보호하는 업적을 세우셨으니 진선으로 불리우시기에 부족함이 없습니다. 지금 바로 승급을 원하시나요?', {
                '예',
                '아뇨',
            })
            if choice == nil or choice ~= 0 then
                return
            end
            local weapon_name = PROMOTION_4TH_WEAPON[me:class()]
            if not weapon_name then
                return
            end
            local code = me:exchange(
                { ['item'] = { ['반고의심장'] = 1 } },
                { ['item'] = { [weapon_name] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 무기를 드릴 수 없습니다.', false, true)
                return
            end
            if q then
                q:complete()
            end
            me:dialog(npc, '축하드립니다. 그에 걸맞는 무기를 지급하였습니다.', false, true)
            return
        end

        local ok, err_msg = meets_4th_promotion_quest_requirements(me)
        if not ok then
            me:dialog(npc, err_msg, false, true)
            return
        end
        local q = me:start_quest(quest.QUEST_MUTA)
        if q == nil then
            me:dialog(npc, '퀘스트를 시작할 수 없습니다.', false, true)
            return
        end
        me:dialog(npc, '기본적인 능력은 갖추셨습니다만, 그만한 지위에 오르시려거든 세상을 위한 업적을 세우셔야 합니다.\n\n소문에, 천인(天人)으로 추앙받고 계신 무타님께서 세상에 닥쳐올 재앙에 대항할 인재를 구하신다고 하던데 한 번 찾아가보시는 것이 어떨까요?', true, true)
        return
    end

    if selected == 1 then
        local weapon_name = PROMOTION_4TH_WEAPON[me:class()]
        if not weapon_name then
            me:dialog(npc, '아직 준비가 안 되었습니다.', false, true)
            return
        end
        if has_4th_weapon(me) then
            me:dialog(npc, '이미 해당 무기를 보유하고 계십니다.', false, true)
            return
        end
        local code = me:exchange(
            { ['money'] = WEAPON_REISSUE_GOLD },
            { ['item'] = { [weapon_name] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '금전 1천만 전이 필요합니다.', false, true)
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 무기를 드릴 수 없습니다.', false, true)
            return
        end
        me:dialog(npc, '무기를 지급하였습니다.', false, true)
    end
end
