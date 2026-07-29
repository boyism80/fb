return {
    group_lock = function(me, group)
        if group == nil then
            me:chat('그룹이 없어용')
            return false
        end

        local map = me:map()
        if map == nil then
            return false
        end

        local nears = group:nears(me:map(), {me:position()})
        for _, near in pairs(nears) do
            near:chat(near:name())
        end
        return true, #nears
    end,

    sample_message = function()
        broadcast('message NOTIFY', MESSAGE_TYPE.NOTIFY)
        broadcast('message BLUE', MESSAGE_TYPE.BLUE)
        broadcast('message STATE', MESSAGE_TYPE.STATE)
        broadcast('message SHOUT', MESSAGE_TYPE.SHOUT)
        broadcast('message WORLD', MESSAGE_TYPE.WORLD)
        broadcast('message POPUP', MESSAGE_TYPE.POPUP)
        broadcast('message YELLOW', MESSAGE_TYPE.YELLOW)
        broadcast('message BROWN', MESSAGE_TYPE.BROWN)
    end,

    -- Protocol probe: uncomment exactly one send line, then:
    --   /스크립트 scripts/script.lua protocol_test
    protocol_test = function(me)
        me:chat('protocol_test: uncomment one packet line in scripts/script.lua')

        -- S2C 0x12 unknown_12 — oid / inventory slot / flag (<0xA0 [무장], >=0xA0 level-up toast)
        -- me:unknown_12()
        -- me:unknown_12(me:oid(), 1, 1)      -- invent slot a → append [무장]
        -- me:unknown_12(me:oid(), 0, 0xA0) -- CharStats self level-up toast

        -- S2C 0x26 move_confirm_noscroll — current direction/position/viewport
        -- me:move_confirm_noscroll()

        -- S2C 0x35 popup_message — long timer (resource 96)
        -- me:popup_message()
        -- me:popup_message('긴 팝업 테스트', 0, 2, 8, 0)

        -- S2C 0x1B popup_input — editable short popup; yields until C2S 0x23 submit
        -- local text = me:popup_input()
        -- local text = me:popup_input('짧은 팝업 테스트', 0, 0, 2, 8)
        -- me:popup_input('짧은 팝업 테스트', 0, 0, 2, 8, { immediate = true })

        -- S2C 0x4B c2s_relay — default echoes C2S miss(0x0C)+self oid
        -- me:c2s_relay()

        -- S2C 0x4D user_info — type 0 empty form; type 2 prefills valid combo labels
        -- me:user_info()
        -- me:user_info(2)
        -- me:user_info(2, '홍길동', '테스트', '비고', '메모', 'A', '서울시', '게임방 및 인터넷카페', '친구')

        -- S2C 0x2F dialog stubs
        -- local idx = me:menu(me, 'menu0', {'선택1', '선택2'})              -- no ext (subtype 0)
        -- local idx = me:menu(me, 'menu1', {'선택1', '선택2'}, { ext = '' }) -- with ext (subtype 1)
        -- local text = me:input(me, 'input2')                               -- no ext (subtype 2)
        -- local text = me:input(me, 'input3', { ext = '' })                 -- with ext (subtype 3)
        -- local name = me:pursuit(me, 'pursuit', {'옵션A', '옵션B'})
        -- local dual = me:dual_field(me, 'dual', {라벨1 = '값1', 라벨2 = '값2'})
        -- local slot = me:spell(me, 'spell')  -- dialog; lookup is me:spell('금강퇴') via life

        -- S2C 0x30 dialog
        -- local act = me:dialog(me, nil, { prev = true, next = true })                              -- subtype 1
        -- local i, btn = me:list(me, nil, {'항목1', '항목2'}, { prev = true })                       -- subtype 3
        -- local s = me:input(me, nil, { top = 'top', bottom = 'bottom', maxlen = 12, prev = true }) -- subtype 5
        -- local s = me:input(me, 'message', { top = 'top', bottom = 'bottom', maxlen = 12, prev = true, password = true }) -- 7
        -- local s = me:input(me, nil, { top = 'top', bottom = 'bottom', maxlen = 12, prev = true, password = true })      -- 8
        -- local s = me:email(me, 'message')
        -- me:dialog_0x30_10(me, 'local look picker') -- needs DLGMSGH.EPF
    end,

    func = function(me, val1, val2, val3)
        local s = me:input(me, nil, { top = 'top', bottom = 'bottom', maxlen = 12, prev = true })
        me:chat(s)
    end
}
