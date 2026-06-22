-- item: 낙랑의두루마리4
-- 낙랑의두루마리4 사용 스크립트
local quest = require('lib.quest')
local enum = require('lib.enum')

local HUNTING_MAPS = {
    ['부여왕초보사냥1'] = true,
    ['장훈왕초보사냥1'] = true,
    ['읍루왕초보사냥1'] = true,
    ['만번한왕초보사냥1'] = true,
    ['천안궁왕초보사냥1'] = true,
    ['상곡왕초보사냥1'] = true,
    ['어양왕초보사냥1'] = true,
    ['관령왕초보사냥1'] = true,
    ['범안왕초보사냥1'] = true,
    ['평양왕초보사냥1'] = true,
    ['졸본왕초보사냥1'] = true,
    ['숙군왕초보사냥1'] = true,
    ['선양왕초보사냥1'] = true,
    ['집안왕초보사냥1'] = true,
    ['산해왕초보사냥1'] = true,
    ['신성왕초보사냥1'] = true,
    ['용성왕초보사냥1'] = true,
    ['후연왕초보사냥1'] = true,
    ['개주왕초보사냥1'] = true,
    ['동부여왕초보사냥1'] = true,
    ['송원왕초보사냥1'] = true,
    ['고구려왕초보사냥1'] = true,
}

function ON_ACTIVATED_4(me, item)
    local q = me:quest(quest.QUEST_NAKRANG4)
    local btn
    local model = item:model()

    if q == nil or q:completed() then
        ::ON_ACTIVATED_4_0000::
        btn = me:dialog(model, '<임무>\n 사냥터 찾아가기\n\n<내용>\n 이제, 목도를 사용할 곳을 찾아가봅시다!!\n본격적인 사냥을~~ 일단 주막에서 나오셔서,\n화면의 왼쪽 위의 조그만 사각형 버튼을\n누르시면 미니맵이 나옵니다.(단축키 *), 이것을\n참고하셔서 \'왕초보사냥터\'로 찾아오세요.\n\n<보상>\n 노란비서, 경험치 100 (레벨5미만)', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_4_0000
        end
        ::ON_ACTIVATED_4_0001::
        btn = me:dialog(model, '<힌트>\n 집 내부에 있으면 미니맵이 안나오는 경우가\n있습니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_4_0000
        end
        if q == nil then
            q = me:start_quest(quest.QUEST_NAKRANG4)
            if q == nil then
                return
            end
        end
        q:step(1)
        return
    end

    local step = q:step()
    if step ~= 1 then
        return
    end

    local map = me:map()
    local map_name = map and map:model() and map:model():name() or ''
    if not HUNTING_MAPS[map_name] then
        ::ON_ACTIVATED_4_0002::
        btn = me:dialog(model, '일단 주막에서 나오셔서,\n화면의 왼쪽 위의 조그만\n사각형 버튼을 누르시면\n미니맵이 나옵니다.(단축키\n*), 이것을 참고하셔서\n\'왕초보사냥터\'로 찾아오세요.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(model, '왕초보사냥터 안에 들어오셔서 두루마리를 사용하셔야 합니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_4_0002
        end
        return
    end

    ::ON_ACTIVATED_4_0003::
    btn = me:dialog(model, '잘 찾아오셨네요!!!', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(model, '잘 하셨습니다!! 상으로\n노란비서를 드릴게요!!', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_4_0003
    end

    local give_exp = (me:level() < 5)
    local code = me:exchange(
        { ['item'] = { ['낙랑의두루마리4'] = 1 } },
        { ['item'] = { ['낙랑의두루마리5'] = 1, ['노란비서'] = 10 }, ['exp'] = give_exp and 100 or 0 }
    )
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(model, '소지품이 가득 찼습니다.', false, false)
        return
    end
    if give_exp then
        me:dialog(model, '<보상>\n\'노란비서\',\n\'낙랑의두루마리5\'을 얻다!!!\n경험치 100 상승!!', false, true)
    else
        me:dialog(model, '<보상>\n\'노란비서\',\n\'낙랑의두루마리5\'을 얻다!!!', false, true)
    end

    q:complete()
end
