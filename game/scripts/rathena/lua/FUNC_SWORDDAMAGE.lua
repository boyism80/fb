{
    set @sd, mysd();
    set @id, myid();

    set @input$, script_input(@sd);
    set @weapon$, get_equipitem_name(@sd, 0);

    set @drate, getarg(0);
    set @spell_id, getarg(1);
    set @spell_delay, getarg(2);
    set @getmp, getarg(3);

    set @min_l, get_l_mindam(@sd); // 공격데미지L 최소값
    set @max_l, get_l_maxdam(@sd); // 공격데미지L 최대값
    set @min_s, get_s_mindam(@sd); // 공격데미지S 최소값
    set @max_s, get_s_maxdam(@sd); // 공격데미지S 최대값
    
    callfunc "MANA_DELAY", @getmp, 0, @sd;
    if(!strcmp(@weapon$, @input$))
    {
        set $damrate, (@drate-1);
        callfunc "MANA_DELAY", 0, @spell_id, @sd;
        message @sd, 3, @weapon$+"이/가 푸른 빛으로 빛납니다.";
        callfunc "BEGINFUNC", 0, 0, @spell_id, @spell_delay, 6, 0;

        set_damrate @sd, get_damrate(@sd)+$damrate;
    }
    else
    {
        message @sd, 3, "마법이 실패했습니다.";
    }
    set_mp @sd, get_mp(@sd)-@getmp;
    return;
}