{
    set @sd, mysd();
    set @id, myid();

    if(clan_exist(@sd) != 0)
    {
        if(get_level(@sd) == 99 && !strcmp(get_clanname(@sd), getarg(1)))
        {
            warp @sd, name2mapid(getarg(0) + "팔괘방"), rand(16, 17), rand(29, 30);
        }
    }
    return;
}