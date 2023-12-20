{
    if(rand(1, getarg(1)) <= getarg(2))
    {
        if(getarg(3) > 0) { dropitem get_mobx(getarg(0)), get_moby(getarg(0)), getarg(3), getarg(4); }
        else { announce 5, get_mobname(getarg(0)) + "몹 드랍이 오류다!"; }
    }
    return;
}