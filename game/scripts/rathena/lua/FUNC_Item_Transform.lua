{
    set @sd, mysd();
    set @id, myid();

    spelluncast @sd, name2spellid("변신시약");
    spelluncast @sd, name2spellid("금수");
    spelluncast @sd, name2spellid("경수");
    spelluncast @sd, name2spellid("야수");
    spelluncast @sd, name2spellid("맹수");
    spelluncast @sd, name2spellid("투명");

    spellcast @sd, name2spellid("변신시약");

    set_disguiselook @sd, getarg(0);
    set_state @sd, 4;
    updatelookpc @sd, 1;

    effect @id, getarg(1);
    sound @id, getarg(2);

    return;
}