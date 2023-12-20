{
    if(getarg(0) == 1 || getarg(0) == 3 || getarg(0) == 5 || getarg(0) == 7 || getarg(0) == 8 || getarg(0) == 10 || getarg(0) == 12)
    {
        return 31;
    }
    else if(getarg(0) == 4 || getarg(0) == 6 || getarg(0) == 9 || getarg(0) == 11)
    {
        return 30;
    }
    else if(getarg(0) == 2)
    {
        return 28;
    }
    return;
}