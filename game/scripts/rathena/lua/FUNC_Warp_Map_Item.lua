{
    set @sd, mysd();
    set @id, myid();

    delitem @sd, getarg(4), 1;
    sound @id, getarg(3);
    warp @sd, getarg(0), getarg(1), getarg(2);
    return;
}