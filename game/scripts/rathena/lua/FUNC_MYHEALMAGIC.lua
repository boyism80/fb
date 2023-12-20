{
    set @id, myid();
    set @sd, mysd();
    
    callfunc "MANA_DELAY", getarg(0), 0, @sd;
    
    if(get_maxhp(@sd) < get_hp(@sd)+getarg(1)) { set_hp @sd, get_maxhp(@sd); showgage @id; }
    else { set_hp @sd, get_hp(@sd)+getarg(1); showgage @id; }
    
    set_mp @sd, get_mp(@sd)-getarg(0);
    callfunc "BEGINFUNC", getarg(2), getarg(3), 0, 0, 6, 0;
    message @sd, 3, getarg(4) + "을(를) 외웠습니다.";
    return;
}