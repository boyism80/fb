{
    set @sd, mysd();
    set @id, myid();
    if(rand(1, 2) == 1) { set @x, rand(3, 6); }
    else { set @x, rand(34, 37); }

    warp @sd, name2mapid("현무의심장"), @x, 6;
}