
local COOLDOWN_SEC = 600
local FLOWER_NAME = "추모국화"
local BALLOON_NAME = "추모풍선"
local BALLOON_CHANCE = 10

local function next_available(me)
    local q = me:quest(QUEST_CHOOMO)
    if q == nil then
        return 0
    end
    local p = q:param()
    if p == nil or p == "" then
        return 0
    end
    return tonumber(p) or 0
end

local function set_cooldown(me)
    local now_ts = now()
    if not now_ts or now_ts <= 0 then
        return
    end
    local q = me:quest(QUEST_CHOOMO)
    if q == nil then
        q = me:start_quest(QUEST_CHOOMO)
        if q == nil then
            return
        end
    end
    if q then
        q:param(tostring(now_ts + COOLDOWN_SEC))
    end
end

function NPC_553(me, npc)
    local sel, btn = me:list(npc, "안녕하신가. 오늘은 슬픈 날이로군.", { "오늘은...", "국화 한 송이를..." }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 0 then
        local d = me:dialog(npc, "오늘은 6월 25일, 비통한 날, 민족의 아픔이 있는 날 이라네...", false, true)
        if d == DIALOG_RESULT.QUIT then
            return
        end
        d = me:dialog(npc, "총탄에 스러진 선인들과, 아직까지 그 아픔을 간직한 채 우리는 살아가고 있지....", false, true)
        if d == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "오늘이 무슨 날인지 한번쯤 생각해 보는 시간을 가졌으면 한다네...", false, false)
        return
    end
    if sel ~= 1 then
        return
    end

    local now_ts = now()
    local next_ts = next_available(me)
    if now_ts and now_ts > 0 and next_ts > now_ts then
        local tick = next_ts - now_ts
        me:dialog(npc, string.format("그렇게 자주 할 수 없네. %d초 후 다시 오게나...", tick), false, false)
        return
    end

    if not me:has_items(FLOWER_NAME, 1) then
        me:dialog(npc, "고인에 대한 진심을 담아 꽃 한송이를 올리게나...", false, false)
        return
    end

    local reward = nil
    if math.random(1, 100) <= BALLOON_CHANCE then
        reward = { ['item'] = { [BALLOON_NAME] = 1 } }
    end
    local code = me:exchange({ ['item'] = { [FLOWER_NAME] = 1 } }, reward)
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "고인에 대한 진심을 담아 꽃 한송이를 올리게나...", false, false)
        return
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 풍선을 받을 수 없네.", false, false)
        return
    end
    set_cooldown(me)
    broadcast(string.format("     << %s님께서 헌화 하셨습니다. >>", me:name()), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
    if reward == nil then
        me:dialog(npc, "자네의 진심을 담은 꽃 한송이를 잘 받았네. 부디 오늘을 잊지 말게나...", false, false)
    else
        me:dialog(npc, "아, 마침 여기에 오늘을 기념하기 위한 풍선이 하나 있었군. 자 받게나...", false, false)
    end
end
