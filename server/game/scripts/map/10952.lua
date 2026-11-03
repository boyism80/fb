-- map: 약수터
-- Spring tiles (54-60, 13): dest=script warps -> on_jungyang_spring

local festival = require('lib.festival')
local quest = require('lib.quest')

local function on_jungyang_spring(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not festival.is('중양절') then
        return
    end
    local q = quest.get_annual(me, quest.QUEST_JUNGYANG_SPRING)
    if q ~= nil and q:completed() then
        return
    end

    local sel, list_btn = me:list(me, '약수를 드시겠습니까?', {
        '네',
        '아니오..',
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel ~= 1 then
        return
    end

    if not festival.is('중양절') then
        return
    end
    q = quest.get_annual(me, quest.QUEST_JUNGYANG_SPRING)
    if q ~= nil and q:completed() then
        return
    end

    if me:mkitem('약신의물병', 1) == nil then
        me:dialog(me, '소지품이 가득 차서 약신의물병을 받을 수 없습니다.', { prev = false, next = false })
        return
    end

    if q == nil then
        q = me:start_quest(quest.QUEST_JUNGYANG_SPRING)
    end
    if q ~= nil then
        q:complete()
        festival.mark_completed_year(q)
    end

    local btn = me:dialog(me, '매달 정해진 한가지씩의 아이템을 모아서 12개를 모두 모으시면 좋은일이 생길꺼에요. 중양절엔 약신의물병입니다.',
        { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
end

return {
    on_jungyang_spring = on_jungyang_spring,
}
