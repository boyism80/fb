-- 낙랑의두루마리8 사용 스크립트
local JOB_MAP_BY_NATION = {
    [NATION.GOGURYEO] = {
        '검성태백',
        '현자해두',
        '태성마로',
        '신선진선',
    },
    [NATION.BUYEO] = {
        '검성예백',
        '현자처연',
        '태성마서',
        '신선선후',
    },
}

local JOB_NAMES = { '전사', '주술사', '도적', '도사' }
local JOB_MASTERS = { '예백', '처연', '마서', '선후' }

local WARP_X_MIN, WARP_X_MAX = 3, 8
local WARP_Y_MIN, WARP_Y_MAX = 5, 8

function ON_ACTIVE_8(me, item)
    local btn
    local model = item:model()

    ::ON_ACTIVE_8_0000::
    btn = me:dialog(model, '<임무>\n 직업가지기\n\n <내용>\n 레벨이 5가 되면 직업을 가져야만 경험치를\n얻으실 수 있습니다. 전사, 주술사, 도적, 도사\n중 마음에 드는 직업을 선택하세요!!!\n\n<보상>\n 새로운두루마리', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0000
    end
    ::ON_ACTIVE_8_0001::
    btn = me:dialog(model, '<힌트>\n 자.. 이제부터 직업에 대한 설명을 드리겠습니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0000
    end
    ::ON_ACTIVE_8_0002::
    btn = me:dialog(model, '<전사>\n 전사는 튼튼한 체력을 바탕으로 그룹 사냥 시\n선두에 서서 방패 역할을 하는 직업입니다.\n전사의 주무기는 칼과 힘이며 그것으로 다른\n직업의 캐릭터들을 보호합니다. 다른 직업에\n비해 사용키의 조작이 비교적 쉽다는 것도\n전사의 장점 중 하나이며, 전사의 단점이라면\n체력이 튼튼한 대신 마력이 약하다는\n것이랍니다. 즉, 전사는 체력이 튼튼한 대신\n순간이동 기술인 소환, 출두, 귀환 등의 마법이\n없으며 위기에 처했을 때 도망가기 어렵습니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0001
    end
    ::ON_ACTIVE_8_0003::
    btn = me:dialog(model, '<주술사>\n 주술사는 강력한 원거리 공격 마법의\n전문가입니다. 주술사는 공격 마법을 많이\n사용하는 특징 때문에 타 직업에 비해서 마력이\n강한 것이 특징입니다. 또한 레벨이 하나 상승할\n때마다 지력이 하나씩 늘어납니다. 그에 비해서\n체력이 매우 낮기 때문에 육탄전에는 적합하지\n않은 직업입니다. 중독, 저주, 마비등의\n효과적인 보조마법과, 강력한 공격마법,\n회복마법을 골고루 사용합니다. 따라서 레벨이\n높아질수록 점점 더 강해지는 직업입니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0002
    end
    ::ON_ACTIVE_8_0004::
    btn = me:dialog(model, '<도적>\n 도적은 기술을 위주로 전투를 하기 때문에\n체력과 마력이 골고루 상승하는 특징을 가지고\n있습니다. 레벨이 하나 상승할 때마다 민첩성이\n하나씩 증가합니다. 또한 체력이 높지 않으므로\n좋은 방어구를 장비할 수 있으며 투명,\n비영승보, 분신 등의 기술을 사용합니다. 투명\n마법은 도적만 가지고 있는 아주 독특한\n기술입니다. 하지만 한 번에 하나의 적만\n상대하기 때문에 사냥 속도가 느립니다. 그런\n단점이 있지만 대신 경험치는 가장 조금 필요한\n직업이라는 특징이 있습니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0003
    end
    btn = me:dialog(model, '<도사>\n 도사는 극히 낮은 체력과 보통의 마력을 가지고\n있습니다. 그리고 레벨이 오를 때마다 지력이\n오릅니다. 도사는 회복 마법의 전문가로 모든\n회복 마법을 배울 수 있습니다. 따라서 그룹\n사냥을 하지 않으면 혼자 전투하기 어렵다는\n단점이 있습니다. 하지만 다시 말하면 회복\n마법을 쓰는 도사가 없이는 그룹 사냥도 별\n의미가 없기도 합니다. 체력을 높여주는 도사는\n그룹사냥에 없어서는 안 될 존재입니다. 보호,\n무장, 태양의 기원, 생명의 기원, 부활 등의\n강력한 지원이 가능하며 강력한 지원군이 됩니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0004
    end

    local sel, list_btn = me:list(model, '자.. 그럼 어떤 직업을 선택할 지\n결정하셧나요? 제가 원하시는 직업을 가질 수 있는 직업 길드로\n보내드릴테니, 원하시는 직업을\n선택해주세요!!', { '전사', '주술사', '도적', '도사', '좀 더 고민해보고 결정할래요' }, false)
    if list_btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == 4 then
        me:dialog(model, '네.. 그럼 심사숙고 해 보신\n뒤 결정하세요..', false, true)
        return
    end
    if sel == nil or sel < 0 or sel > 3 then
        return
    end

    local job_idx = sel + 1
    local jobname = JOB_NAMES[job_idx]
    local jobmaster = JOB_MASTERS[job_idx]

    ::ON_ACTIVE_8_0005::
    btn = me:dialog(model, jobname .. '의 길에 들어서려구요!!\n좋아요. 그럼 ' .. jobname .. ' 길드의\n' .. jobmaster .. '님께 데려다 드릴께요..\n' .. jobname .. '가 되는 방법을\n알려주실 것입니다.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0005
    end
    btn = me:dialog(model, '저의 가르침은 여기까집니다.\n앞으로는 ' .. jobmaster .. '님의 가르침이\n있을 것입니다..\n그럼 안녕히!!!', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_8_0005
    end

    local nation = me:nation()
    local map_names = JOB_MAP_BY_NATION[nation]
    if not map_names then
        map_names = JOB_MAP_BY_NATION[NATION.BUYEO]
    end
    local map_name = map_names[job_idx]
    local map = name2map(map_name)
    if map == nil then
        me:dialog(model, '존재하지 않는 맵입니다.', false, false)
        return
    end

    local x = math.random(WARP_X_MIN, WARP_X_MAX)
    local y = math.random(WARP_Y_MIN, WARP_Y_MAX)
    if x == 6 and y == 6 then
        x = math.random(WARP_X_MIN, WARP_X_MAX)
        y = math.random(WARP_Y_MIN, WARP_Y_MAX)
    end
    me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    me:map(map, { x, y })
end