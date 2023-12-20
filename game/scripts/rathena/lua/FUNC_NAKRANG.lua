{
    set @sd, mysd();
    set @id, myid();
    set @last, lastkillmobid(@sd);

    if($nakrang5 == 1)
    {
        if(@last == name2mobid("다람쥐") && $count_squirrel < 5) { set $count_squirrel, $count_squirrel+1; }
        else if(@last == name2mobid("토끼") && $count_rabbit < 5) { set $count_rabbit, $count_rabbit+1; }
    }
    return;
}