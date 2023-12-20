{
    set @id, myid();
    set @sd, mysd();
    set @hp, getarg(1);
    set @target_id, script_target(@sd);
    
    callfunc "MANA_DELAY", getarg(0), 0, @sd;

    if(istype(@target_id) != 1)
    {
        message @sd, 3, "걸리지 않습니다.";
        end;
    }

    set @target_sd, id2sd(@target_id);

    if(get_state(@target_sd) == 1)
    {
        message @sd, 3, "걸리지 않습니다.";
        end;
    }
    
    if( (get_hp(@target_sd)+@hp) > get_maxhp(@target_sd) ) { set_hp @target_sd, get_maxhp(@target_sd); showgage @target_id; }
    else { set_hp @target_sd, get_hp(@target_sd)+@hp; showgage @target_id; }
    
    set_mp @sd, get_mp(@sd)-getarg(0);
    callfunc "BEGINFUNC", getarg(2), getarg(3), 0, 0, 6, @target_id;
    message @sd, 3, getarg(4) + "을(를) 외웠습니다.";
    if( @id != @target_id )
    {
        message @target_sd, 3, get_name(@sd) + "님이 " + getarg(4) + "을(를) 외워주셨습니다.";
    }
    return;
}