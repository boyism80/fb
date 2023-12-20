{
    set @sd, mysd();
    set @id, myid();

    set @healhp, getarg(0);
    set @healmp, getarg(1);

    if(get_hp(@sd)+@healhp > get_maxhp(@sd)) { set_hp @sd, get_maxhp(@sd); }
    else { set_hp @sd, get_hp(@sd)+@healhp; }

    if(get_mp(@sd)+@healmp > get_maxmp(@sd)) { set_mp @sd, get_maxmp(@sd); }
    else { set_mp @sd, get_mp(@sd)+@healmp; }

    return;
}