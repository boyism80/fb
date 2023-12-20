{
    set @id, getarg(0);
    set @sd, getarg(1);
    set @damage, getarg(2);
    set @hp, getarg(3);
    set @mp, getarg(4);
    set @say$, getarg(5);
    
    if(@mp != 0) { callfunc "MANA_DELAY", @mp, 0, @sd; }

    if($finalattack) { set @damage, @damage + (@damage/2); }
    if($damageup) { set @damage, @damage*2; }

    set @target_id, obj_getfront();
    if(@target_id > 0)
    {
        if(@say$ != "-1") { say @id, 2, @say$; }

        callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @target_id;

        if(istype(@target_id) == 2)
        {
            damage @id, @target_id, @damage, 3;
            set_hp @sd, @hp;
        }
        else if(istype(@target_id) == 1 && ismapoption(@sd, 6) == 1)
        {
            set @r, rand(1, 10);
            set @target_sd, id2sd(@target_id);
            if(@r <= 5 && spellaethered(@target_sd, name2spellid("반탄공")))
            {
                damage @target_id, @id, @damage, 3;
                message @sd, 3, "마법 보호!!";
                end;
            }
            damage @id, @target_id, @damage, 3;
            set_hp @sd, @hp;
        }
        action @id, getarg(8), 30, 0;
        set_mp @sd, get_mp(@sd)-@mp; 
        sound @id, getarg(7);
    }
    return;
}