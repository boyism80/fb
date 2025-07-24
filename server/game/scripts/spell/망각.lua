-- 망각 캐스팅
function ON_CAST_2009(me, you, spell)
    if not you:is(OBJECT_TYPE_MOB) then
        return me:message('걸리지 않습니다.')
    end

    local mp = 60
    local sound = 32
    local effect = 34
    local buff_time = 4
    if buff_cast(me, you, spell, {mp = mp, sound = sound, effect = effect}) then
        you:oblivion(me)
    	you:buff(spell, buff_time, me)
    end
end

-- 망각 버프 효과
function ON_BUFF_2009(me, spell)
    me:target(nil)
end

-- 망각 버프 해제 효과
function ON_UNBUFF_2009(me, spell)
    me:oblivion(nil)
end