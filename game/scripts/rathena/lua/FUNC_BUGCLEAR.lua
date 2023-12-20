{
    set @sd, mysd();
    set @id, myid();
    set @job, get_job(@sd);

    for(set @i, 0; @i < 8; set @i, @i+1)
    {
        if(get_equipitem(@sd, @i) != 0) { set @pos, dialog(0, 0, 0, "아이템을 하나라도 착용하고 있으면 날 만날수 없네!"); end; }
    }

    if(@job == 3)
    {
        set_s_mindam @sd, @job*4;
        set_s_maxdam @sd, @job*4;
        set_l_mindam @sd, @job*4;
        set_l_maxdam @sd, @job*4;
    }
    else if(@job == 4)
    {
        set_s_mindam @sd, @job*3;
        set_s_maxdam @sd, @job*3;
        set_l_mindam @sd, @job*3;
        set_l_maxdam @sd, @job*3;
    }
    else
    {
        set_s_mindam @sd, 0;
        set_s_maxdam @sd, 0;
        set_l_mindam @sd, 0;
        set_l_maxdam @sd, 0;
    }

    set_ac @sd, 100-get_level(@sd);
    set_protection @sd, 0;
    set_healing @sd, 0;
    set_dam @sd, 0;
    set_hit @sd, 0;
    set_healrecycling @sd, 0;
    set_maxhp @sd, get_purehp(@sd);
    set_maxmp @sd, get_puremp(@sd);
    message @sd, 3, "버그해결 완료.";
    return;
}