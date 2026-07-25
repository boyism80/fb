-- mob: 백열장수련도우미

return {
    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_die = function(me)
    -- end,

    -- on_mob_kill = function(me, mobs)
    -- end,

    on_mob_spell_hit = function(me, you, spell)
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
