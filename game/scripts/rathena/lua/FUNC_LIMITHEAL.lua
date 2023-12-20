{
    set @sd, mysd();
    set @id, myid();
    set @delay, spelldelaycount(@sd, getarg(0));
    return @delay;
}