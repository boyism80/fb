-- mob: 백열장수련도우미

return {
    -- ON_MOB_ATTACK = function(me, you)
    -- end,

    -- ON_MOB_DIE = function(me)
    -- end,

    -- ON_MOB_KILL = function(me, mobs)
    -- end,

    ON_MOB_SPELL_HIT = function(me, you, spell)
        me:hp(me:maxhp())
        if spell:name() ~= '백열장' then
            me:chat('백열장이 아닙니다.')
            return false
        end

        me:chat('좋아 좋아!')
        you:chat('좋아 좋아!')
        you:mp(you:maxmp())
        return false
    end
}
