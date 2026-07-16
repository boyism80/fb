-- npc: 전전
local enum = require('lib.enum')

local function try_craft_weapon(me, npc, materials, price, reward_name)
    local materials_table = {}
    for _, m in ipairs(materials) do
        materials_table[m[1]] = m[2]
    end
    local btn = me:dialog(npc, '좋아좋아, 여기있네. 조심해서 좋은 일에 잘 쓰시게나.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    local cost = { ['item'] = materials_table }
    if price > 0 then
        cost['money'] = price
    end
    local code = me:exchange(cost, { ['item'] = { [reward_name] = 1 } })
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '재료가 조금 부족한 것 같은데? 다시 한번 살펴봐.', false, true)
        return false
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 ' .. name_with(reward_name, '은', '는') .. ' 줄 수 없네.', false, true)
        return false
    end
    return true
end

function NPC_167(me, npc)
    local PRICE = 5000
    local SPIDER_FANG = '거미의이빨'
    local SPIDER_WEB = '독거미줄'
    local SPIDER_FANG_COUNT = 25
    local SPIDER_WEB_COUNT = 50
    local SPIDER_SPEAR = '거미독창'
    local WOLF_FANG = '늑대의이빨'
    local WOLF_FANG_COUNT = 50
    local WOLF_FANG_CURVED = '늑대의송곳니'
    local WOLF_FANG_CURVED_COUNT = 10
    local CURSED_DAGGER = '저주의단검'

    ::NPC_167_0001::
    local btn = me:dialog(npc, '안녕, 보시다시피 나는 사냥꾼이야. 요즘 잡스러운 괴물들이 다시 들끓더니 외부인까지 들어오더만\n\n뭐 나쁜 일만은 아니야. 괴물들을 상대하려면 사람들이 많이 필요하거든. 이를테면 자네가 도와준다거나 할 수 있으니까.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_167_0002::
    btn = me:dialog(npc, '아무튼 자네의 두 눈에 정의감이 흘러넘치는걸 보니 괴물들과 싸울 작정인가 보군. 그래, 그럴거야.\n\n그렇다면, 나도 가만히 있을수는 없지. 우리 가문에 대대로 전해져 내려오는 가문의 비밀 무기들을 만들어주지', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_167_0001
    end

    ::NPC_167_0003::
    btn = me:dialog(npc, '각각 거미독창과 저주의단검이라 불리는 무기들일세. 갖고 싶다면 재료만 좀 구해오면 되고... 또 돈도 조금만 주면 끝이지. 후후후후', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_167_0002
    end

    ::NPC_167_0004::
    local list = me:list(npc, '우리 가문 특유의 무기들이야, 골라보게!', { '이 거미독창이란게 마음에 드네요', '이 저주의단검이란게 마음에 들어요', '에이, 별로 좋은건 없어보이는데요.' })
    if list == nil then
        return
    end
    if list == 3 then
        me:dialog(npc, '별로 좋은게 없어보인다니? 한번 사용은 해봤나? 한번 써본다면 그런 말은 나오지 않을걸?', false, true)
        return
    end

    if list == 1 then
        local list_2 = me:list(npc, '탁월한 선택! 지금까지 이 창에 맞고 중독되지 않은 몬스터는 없었지!', { '만들어주세요', '음, 다시 생각해볼래요' })
        if list_2 == nil then
            return
        end
        if list_2 == 2 then
            me:dialog(npc, '그래, 조금 더 생각해보고 오라구.', false, true)
            return
        end
        btn = me:dialog(npc, '거미독창을 만들려면 거미의이빨 25개와 독거미줄 50개가 필요해. 그리고 제작비와 수고비, 세금을 합해 5000전이 필요하지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local list_3 = me:list(npc, '뛰어난 성능에 저렴한 가격! 한 번 믿어봐!', { '어서 만들어 주세요', '나중에 다시 올게요...' })
        if list_3 == nil then
            return
        end
        if list_3 == 2 then
            me:dialog(npc, '또 그세 마음이 바뀐거야? 그럼 나중에 꼭 오라구!', false, true)
            return
        end
        if try_craft_weapon(me, npc, { { SPIDER_FANG, SPIDER_FANG_COUNT }, { SPIDER_WEB, SPIDER_WEB_COUNT } }, PRICE, SPIDER_SPEAR) then
            me:dialog(npc, '수고했네.', false, true)
        end
        return
    end

    if list == 2 then
        local list_2 = me:list(npc, '탁월한 안목! 어떤 적이라해도 저주를 걸어버리는 검이지!', { '만들어주세요', '음, 다시 생각해볼래요' })
        if list_2 == nil then
            return
        end
        if list_2 == 2 then
            me:dialog(npc, '그래, 조금 더 생각해보고 오라구.', false, true)
            return
        end
        btn = me:dialog(npc, '저주의단검을 만들려면 늑대의이빨 50개와 늑대의송곳니 10개가 필요해. 그리고 제작비와 수고비, 양도세를 합해 5000전이 필요하지!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local list_3 = me:list(npc, '엄청난 성능에 비해 너무나 싼 가격! 이 사람 거짓말 안해!', { '좋아요! 만들어주세요!', '사냥꾼이 아니라 장사꾼 같은데요...그만 둘래요.' })
        if list_3 == nil then
            return
        end
        if list_3 == 2 then
            me:dialog(npc, '또 그세 마음이 바뀐거야? 그럼 나중에 꼭 오라구!', false, true)
            return
        end
        if try_craft_weapon(me, npc, { { WOLF_FANG, WOLF_FANG_COUNT }, { WOLF_FANG_CURVED, WOLF_FANG_CURVED_COUNT } }, PRICE, CURSED_DAGGER) then
            me:dialog(npc, '수고했네.', false, true)
        end
        return
    end
end