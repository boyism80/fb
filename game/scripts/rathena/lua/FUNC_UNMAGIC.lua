{
    set @sd, mysd();
    set @id, myid();
    set @target_id, script_target(@sd);
    set @target_sd, id2sd(@target_id);
    
    callfunc "MANA_DELAY", getarg(0), 0, @sd;
    
    set_mp @sd, get_mp(@sd)- getarg(0);
    callfunc "BEGINFUNC", getarg(1), getarg(2), 0, 0, 6, @target_id;
    spelluncast @target_sd, getarg(3);
    message @sd, 3, getarg(4) + "을(를) 외웠습니다.";
    if( @sd != @target_sd )
    {
        message @target_sd, 3, get_name(@sd) + "님이 " + getarg(4) + "을(를) 외워주셨습니다.";
    }
    return;
}