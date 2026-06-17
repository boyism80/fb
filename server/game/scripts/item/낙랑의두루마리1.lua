-- 낙랑의두루마리1 사용 스크립트
function ON_ACTIVATED_1(me, item)
    ::NAK_1::
    local btn = me:dialog(item, '벌써 두루마리 사용법을 익히셨군요!! 잘 하셨어요!! 그럼 제가 주막으로 보내드릴께요...', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NAK_2::
    btn = me:dialog(item, '주막에서 두루마리를 다시 열어보세요... 꼭이요!!', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NAK_1
    end
    btn = me:dialog(item, '그럼... 뿅~', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NAK_2
    end

    local maps = nil
    if me:nation() == NATION.GOGURYEO then
        maps = { 29, 30, 31, 8016, 8017, 8018, 8043, 8044, 8045, 8070, 8071, 8072, 8097, 8098, 8099, 8124, 8125, 8126, 8151, 8152, 8153, 8178, 8179, 8180, 8205, 8206, 8207, 8232, 8233, 8234, 8259, 8260, 8261 }
    else
        maps = { 15, 16, 17, 7716, 7717, 7718, 7743, 7744, 7745, 7770, 7771, 7772, 7797, 7798, 7799, 7824, 7825, 7826, 7851, 7852, 7853, 7878, 7879, 7880, 7905, 7906, 7907, 7932, 7933, 7934, 7959, 7960, 7961 }
    end

    math.randomseed(seed())
    local map = maps[math.random(1, #maps)]
    me:map(map, nil, {
        callback = function()
            me:exchange(
                { ['item'] = { ['낙랑의두루마리1'] = 1 } },
                { ['item'] = { ['낙랑의두루마리2'] = 1, ['노란비서'] = 1, ['도톨'] = 2 } }
            )
        end,
    })
end