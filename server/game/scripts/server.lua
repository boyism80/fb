-- C++ entry points: F1/F2 menu, sky maze warps, 길림신전 warp (see click.cpp, move.cpp).
local enum = require('lib.enum')
local storage = require('lib.storage')
local marketplace = require('lib.marketplace')
local matchmaking = require('lib.matchmaking')
local grant_storage = require('lib.grant_storage')
local sky_maze = require('lib.sky_maze')

return {
    on_f1_event = function(me)
        local npc = name2npc('낙랑')

    ::F1_MENU::
        local menu = {'통합보관함', '거래소', '매치메이킹'}
        if me:role() >= ROLE.ADMIN then
            table.insert(menu, '아이템 지급')
        end

        local selected = me:list(npc, '무엇을 도와드릴까요?', menu)
        if selected == nil then
            return
        end

        if selected == 1 then
            if storage.handle(me, npc) == false then
                return
            end
        elseif selected == 2 then
            if marketplace.handle(me, npc) == false then
                return
            end
        elseif selected == 3 then
            if matchmaking.handle(me, npc) == false then
                return
            end
        elseif selected == 4 and me:role() >= ROLE.ADMIN then
            if grant_storage.handle(me, npc) == false then
                return
            end
        end

        goto F1_MENU
    end,

    on_f2_event = function(me)
        local npc = name2npc('낙랑')
        local button = me:dialog(npc, 'F2 이벤트 스크립트', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end,

    ON_WARP_SKY_MAZE = function(me)
        return sky_maze.warp(me)
    end,

    ON_WARP_PK_SKY_MAZE = function(me)
        return sky_maze.warp_pk(me)
    end,

    ON_MAP_GUILLIM_TEMPLE = function(me)
        local reward_by_offer = {
            ['산의옥'] = '고대마법서\'상',
            ['구름옥'] = '서문열쇠',
            ['불의옥'] = '고대마법서\'하',
            ['물의옥'] = '동문열쇠',
        }
        local option_names = {'산의옥', '구름옥', '불의옥', '물의옥'}
        local selected, button = me:dialog(nil, '인간이여.. 여기엔 무슨일로 왔는가..', {'말씀을 들으러 왔습니다.', '제물을 바치러 왔습니다.'})
        if selected == 1 then
            return
        end

        if selected == 2 then
            selected, button = me:dialog(nil, '허허.. 그래. 무엇을 제물로 바치겠느냐?', {'산의옥', '구름옥', '불의옥', '물의옥'})
            if selected == nil then
                return
            end
            if selected < 1 or selected > 4 then
                return
            end
            local offer_name = option_names[selected]
            local reward_name = reward_by_offer[offer_name]
            if reward_name == nil then
                return
            end
            if name2item(offer_name) == nil or name2item(reward_name) == nil then
                return
            end
            if not me:has_items(offer_name, 1) then
                me:dialog(nil, '지금 감히 난에게 거짓말을 하는 것이냐! 썩 물러가거라.', { prev = false, next = true })
                me:map(name2map('길림성'), 0, 0)
                return
            end

            me:dialog(nil, '기특한 인간이여.. 내 그대에게 선물을 주겠노라..', { prev = false, next = true })
            local code = me:exchange(
                { ['item'] = { [offer_name] = 1 } },
                { ['item'] = { [reward_name] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(nil, '지금 감히 난에게 거짓말을 하는 것이냐! 썩 물러가거라.', { prev = false, next = true })
                me:map(name2map('길림성'), 0, 0)
                return
            end
            if code ~= enum.EXCHANGE_RESULT.OK then
                return
            end
            if selected == 1 then
                me:dialog(nil, '이것은 고대에서 전해지는 마법서로.. 두권중 첫번째 권이다.', { prev = false, next = true })
                me:dialog(nil, '두번째 책은 자네가 나를 위해서 좀 더 일해 준다면 선물로 주도록 하겠다.', { prev = false, next = true })
                me:dialog(nil, '동쪽의 어둠을 처치하였으니.. 이번엔 북쪽의 미로를 찾아 가보거라.', { prev = false, next = true })
            elseif selected == 2 then
                me:dialog(nil, '이것은 길림성 서쪽에 있는 태극문의 열쇠다.', { prev = false, next = true })
                me:dialog(nil, '이 열쇠로 태극문으로 들어가서 마지막 암흑왕의 존재를 물리치고 오너라.', { prev = false, next = true })
            elseif selected == 3 then
                me:dialog(nil, '내가 그대에게 준 마법서 두권은 오래전에 이 세계가 암흑으로 둘러싸여 있을때 그것을 물리친 마법사가 암흑을 봉인한 위치를 적어놓은 것이다.', { prev = false, next = true })
                me:dialog(nil, '고대어로 쓰여있어서 읽기가 힘드니, 너에게 이 임무를 준 성현을 찾아가도록 하여라.', { prev = false, next = true })
                me:dialog(nil, '그리고 언제나 내가 너에게 힘을 주고 있다는 사실을 잊지말도록 하여라.', { prev = false, next = true })
            elseif selected == 4 then
                me:dialog(nil, '이것은 길림성 동쪽에 있는 태극문의 열쇠다.', { prev = false, next = true })
                me:dialog(nil, '이 열쇠를 가지고 가면 태극문으로 들어갈 수 있을 것인데..')
                me:dialog(nil, '그곳에서 또 다른 암흑의 존재를 물리치고 오너라.', { prev = false, next = true })
            end
        end
    end
}
