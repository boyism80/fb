
local YUT_COST = 10

local function yut_result()
    local r = math.random(1, 16)
    if r == 1 then
        return "도", false
    elseif r >= 2 and r <= 4 then
        return "개", false
    elseif r >= 5 and r <= 10 then
        return "걸", false
    elseif r >= 11 and r <= 14 then
        return "윷", true
    elseif r == 15 then
        return "모", true
    else
        return "빽도", false
    end
end

function NPC_286(me, npc)
    if me:money() < YUT_COST then
        npc:chat(string.format("%s: [%s]님. 금전이 부족하시네요. 10전을 가져오세요.", npc:model():name(), me:name()))
        return
    end

    me:money(me:money() - YUT_COST)
    local pae, again = yut_result()
    local plus = again and " 한 번 더 던지세요." or ""
    npc:chat(string.format("%s: [%s]님. %s 나왔습니다.%s", npc:model():name(), me:name(), pae, plus))
end
