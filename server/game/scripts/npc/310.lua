-- npc: 오엑스도우미
local npc = require('lib.npc')
local enum = require('lib.enum')

local function run_ox_admin(me, ch)
    if not me:has_items('아무나OX주최토큰', 1) then
        return me:dialog(ch, '아무나OX주최토큰이 없으시군요.', { prev = false, next = true })
    end

    local sub_sel, sub_btn = me:list(ch, '어떤 명령을 수행하시겠습니까?', {
        '출입구열기',
        '출입구닫기',
        '추방',
    }, { prev = false })
    if sub_btn == DIALOG_RESULT.QUIT or sub_sel == nil then
        return DIALOG_RESULT.NEXT
    end

    if sub_sel == 1 then
        local m = me:map()
        m:block(7, 13, false)
        m:block(21, 13, false)
        return me:dialog(ch, '출입구를 열었습니다.', { prev = false, next = true })
    elseif sub_sel == 2 then
        local m = me:map()
        m:block(7, 13, true)
        m:block(21, 13, true)
        return me:dialog(ch, '출입구를 닫았습니다.', { prev = false, next = true })
    end

    local raw = me:input(ch, '강제퇴장할 유저의 아이디를 적어주세요.', { top = '퇴장할 대상의 아이디는:', bottom = '입니다.', maxlen = 6, prev = false })
    if raw == DIALOG_RESULT.QUIT then
        return nil
    end
    if raw == nil or raw == '' then
        return me:dialog(ch, '대상 아이디를 입력해 주세요.', { prev = false, next = true })
    end

    local target = name2ch(raw)
    if target == nil then
        return me:dialog(ch, '해당 유저가 접속중이 아닙니다.', { prev = false, next = true })
    end
    if me:map() ~= target:map() then
        return me:dialog(ch, '그럴 수 없어 뿍짝아...', { prev = false, next = true })
    end

    local btn = me:dialog(ch, raw .. '님이 강제 퇴장 됩니다..', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return nil
    end

    local exit_map = name2map('부여성')
    if exit_map then
        target:map(exit_map, 75, 140)
    end
    return DIALOG_RESULT.NEXT
end

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '무엇을 도와드릴까요?',
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
                { '사용자 오엑스', function(me, ch)
                    return run_ox_admin(me, ch)
                end },
                { '토큰 구입', function(me, ch)
                    local code = me:exchange(
                        { ['item'] = { ['도깨비방망이'] = 1 } },
                        { ['item'] = { ['아무나OX주최토큰'] = 1 } }
                    )
                    if code == enum.exchange_result.LACK_COST then
                        return me:dialog(ch, '도깨비방망이가 없으시군요.', { prev = false, next = true })
                    end
                    if code == enum.exchange_result.LACK_CAPACITY then
                        return me:dialog(ch, '소지품이 가득 차서 아무나OX주최토큰을 받을 수 없습니다.', { prev = false, next = true })
                    end
                    return me:dialog(ch, "도깨비방망이를 '아무나OX주최토큰'으로 바꿔드리겠습니다.", { prev = false, next = true })
                end },
            },
        })
    end
}
