{
    set @id, myid();
    set @sd, mysd();
    set @x, get_x(@target_id);
    set @y, get_y(@target_id);
    set @target_id, script_target(@sd);
    
    callfunc "MANA_DELAY", getarg(0), 0, @sd;

    if(istype(@target_id) != 1)
    {
        message @sd, 3, "걸리지 않습니다.";
        end;
    }

    setarray @target_x[0], @x+1, @x  , @x-1, @x;
    setarray @target_y[0], @y  , @y+1, @y  , @y-1;

    for(set @loop, 0; @loop < 4; set @loop, @loop+1)
    {
        set @target_id, obj_getxy(@target_x[@loop], @target_y[@loop]);
        if(@target_id > 0)
        {
            if(istype(@target_id) == 1)
            {
                set @target_sd, id2sd(@target_id);
                if(get_state(@target_sd) != 1)
                {
                    if( (get_hp(@target_sd)+getarg(1)) > get_maxhp(@target_sd) ) { set_hp @target_sd, get_maxhp(@target_sd); showgage @target_id; }
                    else { set_hp @target_sd, get_hp(@target_sd)+getarg(1); showgage @target_id; }
                    callfunc "BEGINFUNC", getarg(2), 0, 0, 0, 0, @target_id;
                    if(@id != @target_id)
                    {
                        message @target_sd, 3, get_name(@sd) + "님이 " + getarg(4) + "을(를) 외워주셨습니다.";
                    }
                }
            }
            else
            {
                set @notarget, @notarget+1;
            }
        }
    }

    if(@notarget < 4)
    {
        callfunc "BEGINFUNC", 0, getarg(3), 0, 0, 6, @id;
    }
    
    set_mp @sd, get_mp(@sd)-getarg(0);
    message @sd, 3, getarg(4) + "을(를) 외웠습니다.";

    deletearray @target_x[0], getarraysize(@target_x);
    deletearray @target_y[0], getarraysize(@target_y);
    return;
}