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
        
        -- me:web_map()                         -- 현재 맵 웹맵 마커
        
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
    end,
    
    -- Interactive dialog suite:
    --   /스크립트 scripts/script.lua dialog_test
    -- Walk each case; press PREV/NEXT/Select/QUIT as prompted and check the chat log.
    -- Client TOP on 0x2F restarts NPC click (releases dialog) — it does not resume here.
    dialog_test = function(me)
        local function result_name(v)
            if v == nil then
                return 'nil'
            end
            if type(v) == 'number' then
                if v == DIALOG_RESULT.PREV then return 'PREV' end
                if v == DIALOG_RESULT.QUIT then return 'QUIT' end
                if v == DIALOG_RESULT.NEXT then return 'NEXT' end
                return tostring(v)
            end
            return tostring(v)
        end
        
        local function log(label, a, b)
            if b ~= nil then
                me:chat(string.format('[dialog_test] %s => %s, %s', label, result_name(a), result_name(b)))
            else
                me:chat(string.format('[dialog_test] %s => %s', label, result_name(a)))
            end
        end
        
        local function pause(msg)
            local r = me:dialog(me, msg, { prev = false, next = true })
            log('pause', r)
            return r ~= DIALOG_RESULT.QUIT
        end
        
        me:chat('[dialog_test] start — follow on-screen prompts')
        
        ------------------------------------------------------------------
        -- 0x30: PREV / NEXT enable + QUIT
        ------------------------------------------------------------------
        if not pause('0x30 dialog: NEXT only (PREV disabled). Press NEXT or QUIT.') then return end
        log('dialog next-only', me:dialog(me, 'NEXT only', { prev = false, next = true }))
        
        if not pause('0x30 dialog: PREV+NEXT enabled. Try PREV, NEXT, QUIT.') then return end
        log('dialog prev+next', me:dialog(me, 'PREV and NEXT', { prev = true, next = true }))
        
        if not pause('0x30 dialog: both disabled (QUIT only).') then return end
        log('dialog none', me:dialog(me, 'no nav buttons', { prev = false, next = false }))
        
        if not pause('0x30 dialog nil message (subtype 1): PREV+NEXT.') then return end
        log('dialog nil-msg', me:dialog(me, nil, { prev = true, next = true }))
        
        ------------------------------------------------------------------
        -- 0x30 list: PREV enable + selection index
        ------------------------------------------------------------------
        if not pause('0x30 list: PREV off. Select an item or QUIT.') then return end
        do
            local i, btn = me:list(me, 'list prev=false', { 'alpha', 'beta', 'gamma' }, { prev = false })
            log('list prev=false', i, btn)
        end
        
        if not pause('0x30 list: PREV on. Try PREV, select, QUIT.') then return end
        do
            local i, btn = me:list(me, 'list prev=true', { 'one', 'two' }, { prev = true })
            log('list prev=true', i, btn)
        end
        
        if not pause('0x30 list nil message (subtype 3).') then return end
        do
            local i, btn = me:list(me, nil, { 'x', 'y' }, { prev = true })
            log('list nil-msg', i, btn)
        end
        
        ------------------------------------------------------------------
        -- 0x30 input_ext: PREV + password variants
        ------------------------------------------------------------------
        if not pause('0x30 input: PREV on. Submit text or PREV/QUIT.') then return end
        do
            local s = me:input(me, 'input_ext message', {
                top = 'top caption',
                bottom = 'bottom caption',
                maxlen = 12,
                prev = true,
            })
            log('input_ext', s)
        end
        
        if not pause('0x30 input nil message (subtype 5): PREV off.') then return end
        do
            local s = me:input(me, nil, {
                top = 'top',
                bottom = 'bottom',
                maxlen = 12,
                prev = false,
            })
            log('input_ext_nil_msg', s)
        end
        
        if not pause('0x30 input password (subtype 7).') then return end
        do
            local s = me:input(me, 'password with message', {
                top = 'password',
                bottom = '',
                maxlen = 12,
                prev = true,
                password = true,
            })
            log('input_password', s)
        end
        
        if not pause('0x30 input password nil message (subtype 8).') then return end
        do
            local s = me:input(me, nil, {
                top = 'top',
                bottom = 'bottom',
                maxlen = 12,
                prev = false,
                password = true,
            })
            log('input_password_nil_msg', s)
        end
        
        ------------------------------------------------------------------
        -- 0x30 email
        ------------------------------------------------------------------
        if not pause('0x30 email. Submit or close.') then return end
        log('email', me:email(me, 'email dialog'))
        
        ------------------------------------------------------------------
        -- 0x2F menu / input
        ------------------------------------------------------------------
        if not pause('0x2F menu (no ext): Select or QUIT. TOP restarts click.') then return end
        do
            local i, btn = me:menu(me, 'menu0', { 'opt_a', 'opt_b' })
            log('menu0', i, btn)
        end
        
        if not pause('0x2F menu (with ext).') then return end
        do
            local i, btn = me:menu(me, 'menu1', { 'opt_a', 'opt_b' }, { ext = '' })
            log('menu1', i, btn)
        end
        
        if not pause('0x2F input (no ext): type text / QUIT.') then return end
        do
            local s, btn = me:input(me, 'input2')
            log('input2', s, btn)
        end
        
        if not pause('0x2F input (with ext).') then return end
        do
            local s, btn = me:input(me, 'input3', { ext = '' })
            log('input3', s, btn)
        end
        
        ------------------------------------------------------------------
        -- 0x2F pursuit / dual / item / slot / spell
        ------------------------------------------------------------------
        if not pause('0x2F pursuit (string list): Select name or QUIT.') then return end
        do
            local OPT = { a = 'option_a', b = 'option_b', c = 'option_c' }
            local name, btn = me:pursuit(me, 'pursuit list', { OPT.a, OPT.b, OPT.c })
            log('pursuit', name, btn)
        end
        
        if not pause('0x2F pursuit dual-field (ordered pairs): Select label or QUIT.') then return end
        do
            local label, btn = me:pursuit(me, 'pursuit dual', {
                { 'HP', '100' },
                { 'MP', '50' },
                { 'ATK', '12' },
            })
            log('pursuit_dual', label, btn)
        end
        
        if not pause('0x2F item: needs shop-like pairs. Select or QUIT.') then return end
        do
            local pairs = {
                { '도토리', 100 },
                { '목검', 500 },
            }
            local name, btn = me:item(me, 'item dialog', pairs)
            log('item', name, btn)
        end
        
        if not pause('0x2F slot: uses your inventory slots. Select or QUIT.') then return end
        do
            local slots = {}
            for slot, _ in pairs(me:items() or {}) do
                table.insert(slots, slot)
            end
            if #slots == 0 then
                me:chat('[dialog_test] slot skipped (empty inventory)')
            else
                table.sort(slots)
                local s, btn = me:slot(me, 'slot dialog', slots)
                log('slot', s, btn)
            end
        end
        
        if not pause('0x2F spell: local spell slots. Select or QUIT.') then return end
        do
            local slot, btn = me:spell(me, 'spell dialog')
            log('spell', slot, btn)
        end
        
        me:chat('[dialog_test] done')
        me:dialog(me, 'dialog_test complete. Check chat for [dialog_test] results.', { prev = false, next = true })
    end,
    
    func = function(me, val1, val2, val3)
        local npc = name2npc('낙랑')
        local button = me:dialog(npc, '대단히 중요하니 끝까지 읽어주세요! 빈번히 발생하는 아이디 해킹을 미연에 방지하기 위해 또 하나의 2차 비밀번호를 정해야 합니다.', { prev = false, next = true })
    end
}
