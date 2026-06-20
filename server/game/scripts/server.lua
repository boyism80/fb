-- C++ entry points: F1/F2 menu, sky maze warps (see click.cpp, move.cpp).
local storage = require('lib.storage')
local marketplace = require('lib.marketplace')
local sky_maze = require('lib.sky_maze')

function ON_F1_EVENT(me)
    local npc = name2npc('낙랑')

::F1_MENU::
    local selected = me:list(npc, '무엇을 도와드릴까요?', {'통합보관함', '거래소'})
    if selected == nil then
        return
    end

    if selected == 0 then
        if storage.handle(me, npc) == false then
            return
        end
    elseif selected == 1 then
        if marketplace.handle(me, npc) == false then
            return
        end
    end

    goto F1_MENU
end

function ON_F2_EVENT(me)
    local npc = name2npc('낙랑')
    local button = me:dialog(npc, 'F2 이벤트 스크립트', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end

function ON_WARP_SKY_MAZE(me)
    return sky_maze.warp(me)
end

function ON_WARP_PK_SKY_MAZE(me)
    return sky_maze.warp_pk(me)
end
