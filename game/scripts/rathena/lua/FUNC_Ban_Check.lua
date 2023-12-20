{
    set @sd, mysd();
    set @id, myid();
    set @nt, callfunc("Server_Nowtime");

    if(get_level(@sd) >= 5)
    {
        if($ban_time > @nt)
        {
            getoutofhere @sd;
            end;
        }
    }
    return;
}