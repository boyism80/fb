{
    set @sd, mysd();
    set @id, myid();

    if(getarg(4) != 0) { set @four, getarg(4); }
    else { set @four, 6; }

    if(getarg(0) != 0)
    {
        if(getarg(5) != 0) { set @smyid, getarg(5); }
        else { set @smyid, @id; }

        effect @smyid, getarg(0);
    }

    if(getarg(1) != 0)
    {
        action @id, @four, 30, 0;
        sound @id, getarg(1);
    }

    if(getarg(2) != 0)
    {
        spellaether @sd, getarg(2), getarg(3);
    }
    return;
}