local M = {}

local FOX_FLOOR2 = '여우에게 포위됐다! 위로 전진해 이곳을 벗어나야겠어.'
local FOX_FLOOR3 = '불여우들 저 너머에 구미호가 보이는군... 위험하니 어서 무찔러야해!'
local FOX_EXIT = {
    '휴... 무서웠다.',
    '무사히 탈출했어!',
    '구미호는 정말 강력하군...',
    '여우가 이렇게 강하다니!!',
    '여우같은 것들... 지긋지긋해!',
}
local FOX_GHOST = '앗, 성황할머니께서 계시네! 살려달라고 말씀드려야지...'

local TIGER_FLOOR1 = '호랑이굴에 들어와도 정신만 차리면 돼!'
local TIGER_FLOOR2 = '많은 문이 보이네... 올바른 길은 어디일까?'
local TIGER_FLOOR3 = '붉은 호랑이의 울음소리가 여기까지 들리는군...'

local MANTIS_1 = '지하에 숨은 괴물을 다 제거할수는 없어... 수풀만 제거하고 이동해야겠어.'
local MANTIS_2 = '죽여도 죽여도 끝이 없을 것 같군! 대체 원인이 뭐지?'
local MANTIS_3 = '험악한 괴물들을 처치하면, 길이 열릴것 같군..'

function M.on_enter(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    local map = me:map()
    if map == nil then
        return
    end

    local name = map:name()
    local n = tonumber(name:match('(%d+)$'))
    if n == nil then
        return
    end

    local r = n % 3
    if name:find('여우굴', 1, true) then
        if r == 2 then
            me:chat(FOX_FLOOR2)
        elseif r == 0 then
            me:chat(FOX_FLOOR3)
        elseif me:state() == STATE.GHOST then
            me:chat(FOX_GHOST)
        else
            me:chat(FOX_EXIT[math.random(#FOX_EXIT)])
        end
    elseif name:find('자호굴', 1, true) then
        if me:state() == STATE.GHOST and r == 1 then
            me:chat(FOX_GHOST)
        elseif r == 1 then
            me:chat(TIGER_FLOOR1)
        elseif r == 2 then
            me:chat(TIGER_FLOOR2)
        else
            me:chat(TIGER_FLOOR3)
        end
    elseif name:find('사마귀굴', 1, true) then
        if r == 1 then
            me:chat(MANTIS_1)
        elseif r == 2 then
            me:chat(MANTIS_2)
        else
            me:chat(MANTIS_3)
        end
    end
end

return M
