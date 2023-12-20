{
    set @sd, mysd();
    set @id, myid();

    if(getarg(0) != 0)
    {
        if(get_mp(@sd) < getarg(0))
        {
            message @sd, 3, "마력이 부족합니다.";
            end;
        }
    }
    
    if(getarg(1) != 0)
    {
        if(spellaethered(getarg(2), getarg(1)) == 1)
        {
            message @sd, 3, "이미 걸려있습니다.";
            end;
        }
    }
    return;
}