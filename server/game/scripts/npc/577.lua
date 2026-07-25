-- npc: 고구려경품상
local enum = require('lib.enum')
local NORMAL_EXCHANGE_REWARDS = {
    { "장안성비서", 5 },
    { "부여성비서", 5 },
    { "국내성비서", 5 },
    { "신부여성비서", 1 },
    { "백세주", 1 },
}

local function run_normal_exchange(me, npc)
    local sel, btn = me:list(npc, "일반교환권을 제가 가지고 있는 아이템과 교환 해 드려요.", {
        "예 교환해 주세요.",
        "아니오. 싫어요."
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel ~= 1 then
        return
    end
    local idx = math.random(1, #NORMAL_EXCHANGE_REWARDS)
    local entry = NORMAL_EXCHANGE_REWARDS[idx]
    local code = me:exchange(
        { ['item'] = { ["일반교환권"] = 1 } },
        { ['item'] = { [entry[1]] = entry[2] } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "일반교환권이 없으시군요. 일반교환권을 가져오세요.", { prev = false, next = false })
        return
    elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 보상을 드릴 수 없습니다.", { prev = false, next = false })
        return
    end
    me:dialog(npc, "일반교환권를 제가 가진 [" .. entry[1] .. "]으로 교환해 드렸어요.", { prev = false, next = false })
end

local SPECIAL_EXCHANGE_REWARDS = {
    { "부여성비서", 50 },
    { "장안성비서", 5 },
    { "부여성비서", 5 },
    { "국내성비서", 5 },
    { "신부여성비서", 1 },
    { "백세주", 1 },
    { "도삭산400층비서", 3 },
    { "도삭산300층비서", 3 },
    { "의태시약10", 3 },
    { "의태시약10", 1 },
    { "정령인형", 1 },
    { "부활시약", 1 },
    { "맑은시약", 1 },
    { "이백세주", 1 },
    { "오백세주", 1 },
    { "삼백세주", 1 },
    { "장인의돌", 1 },
    { "건괘", 1 },
    { "녹호박장갑", 1 },
    { "녹호박팔찌", 1 },
}

local function run_special_exchange(me, npc)
    local sel, btn = me:list(npc, "특별교환권을 제가 가지고 있는 아이템과 교환 해 드려요.", {
        "예 교환해 주세요.",
        "아니오. 싫어요."
    }, { prev = false })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel ~= 1 then
        return
    end
    local idx = math.random(1, #SPECIAL_EXCHANGE_REWARDS)
    local entry = SPECIAL_EXCHANGE_REWARDS[idx]
    local code = me:exchange(
        { ['item'] = { ["특별교환권"] = 1 } },
        { ['item'] = { [entry[1]] = entry[2] } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "특별교환권이 없으시군요. 특별교환권을 가져오세요.", { prev = false, next = false })
        return
    elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 보상을 드릴 수 없습니다.", { prev = false, next = false })
        return
    end
    me:dialog(npc, "특별교환권를 제가 가진 [" .. entry[1] .. "]으로 교환해 드렸어요.", { prev = false, next = false })
end

return {
    ON_CLICK = function(me, npc)
        local map = me:map()
        local map_name = (map and map:model()) and map:model():name() or ""

        if map_name == "고구려경품상2" then
            run_normal_exchange(me, npc)
            return
        end
        if map_name == "고구려경품상3" then
            run_special_exchange(me, npc)
            return
        end
        if map_name == "고구려경품상4" then
            me:dialog(npc, ".............", { prev = false, next = false })
            return
        end
    end
}
