-- 귀환 캐스팅
function ON_CAST_5(me, spell)
    local sound = 33
    local mp = 30
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        return me:message(error)
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    local maps = nil
    if me:nation() == NATION_GOGURYEO then
        maps = {29, 30, 31, 8016, 8017, 8018, 8043, 8044, 8045, 8070, 8071, 8072, 8097, 8098, 8099, 8124, 8125, 8126, 8151, 8152, 8153, 8178, 8179, 8180, 8205, 8206, 8207, 8232, 8233, 8234, 8259, 8260, 8261}
    else
        maps = {15, 16, 17, 7716, 7717, 7718, 7743, 7744, 7745, 7770, 7771, 7772, 7797, 7798, 7799, 7824, 7825, 7826, 7851, 7852, 7853, 7878, 7879, 7880, 7905, 7906, 7907, 7932, 7933, 7934, 7959, 7960, 7961}
    end

    math.randomseed(seed())
    local i = math.random(1, #maps)
    local map = maps[i]
    error = me:map(map)
    if error ~= nil then
        return me:message(error)
    end

    me:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
end