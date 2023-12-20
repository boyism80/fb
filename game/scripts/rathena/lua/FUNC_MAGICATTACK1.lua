{
    set @id, getarg(0);
    set @sd, getarg(1);
    set @damage, getarg(2);
    set @hp, getarg(3);
    set @mp, getarg(4);
    set @say$, getarg(5);
    set @play, 0;
    
    if(@mp != 0) { callfunc "MANA_DELAY", @mp, 0, @sd; }

    switch(get_side(@id))
    {
    case 0:
        set @x, get_x(@id)-1;
        set @y, get_y(@id)-2;
        break;
    case 1:
        set @x, get_x(@id)+2;
        set @y, get_y(@id)-1;
        break;
    case 2:
        set @x, get_x(@id)-1;
        set @y, get_y(@id)+2;
        break;
    case 3:
        set @x, get_x(@id)-2;
        set @y, get_y(@id)-1;
        break;
    }
    
    if($finalattack) { set @damage, @damage + (@damage/2); }
    if($damageup) { set @damage, @damage*2; }

    set @target_id, obj_getfront();
    if(@target_id > 0)
    {
        if(istype(@target_id) == 2)
        {
            callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @target_id;
            damage @id, @target_id, @damage, 3;
        }
        else if(istype(@target_id) == 1 && ismapoption(@sd, 6) == 1)
        {
            set @r, rand(1, 10);
            set @target_sd, id2sd(@target_id);
            if(@r <= 5 && spellaethered(@target_sd, name2spellid("반탄공")))
            {
                callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @id;
                damage @target_id, @id, @damage, 3;
                message @sd, 3, "마법 보호!!";
                goto NEXT;
            }
            callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @target_id;
            damage @id, @target_id, @damage, 3;
        }
NEXT:
        set @play, @play+1;
    }
    for(set @i, 0; @i < 3; set @i, @i+1)
    {
        if(@side == 0 || @side == 2)
        {
            if((@x+@i >= mapwidth() || @x < 0) || (@y >= mapheight() || @y < 0 ))
            {
                continue;
            }
            set @target_id, obj_getxy(@x+@i, @y);
            if(@target_id > 0)
            {
                if(istype(@target_id) == 2)
                {
                    callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @target_id;
                    damage @id, @target_id, @damage, 3;
                }
                else if(istype(@target_id) == 1 && ismapoption(@sd, 6) == 1)
                {
                    set @r, rand(1, 10);
                    set @target_sd, id2sd(@target_id);
                    if(@r <= 5 && spellaethered(@target_sd, name2spellid("반탄공")))
                    {
                        callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @id;
                        damage @target_id, @id, @damage, 3;
                        message @sd, 3, "마법 보호!!";
                        goto NEXT1;
                    }
                    callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @target_id;
                    damage @id, @target_id, @damage, 3;
                }
                else
                {
                    set_mp @sd, get_mp(@sd)-@mp;
                }
NEXT1:
                set @play, @play+1;
            }
        }
        else
        {
            if((@x >= mapwidth() || @x < 0) || (@y >= mapheight() || @y-@i < 0 ))
            {
                continue;
            }
            set @target_id, obj_getxy(@x, @y+@i);
            if(@target_id > 0)
            {
                if(istype(@target_id) == 2)
                {
                    callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @target_id;
                    damage @id, @target_id, @damage, 3;
                }
                else if(istype(@target_id) == 1 && ismapoption(@sd, 6) == 1)
                {
                    set @r, rand(1, 10);
                    set @target_sd, id2sd(@target_id);
                    if(@r <= 5 && spellaethered(@target_sd, name2spellid("반탄공")))
                    {
                        callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @id;
                        damage @target_id, @id, @damage, 3;
                        message @sd, 3, "마법 보호!!";
                        goto NEXT2;
                    }
                    callfunc "BEGINFUNC", getarg(6), 0, 0, 0, 0, @target_id;
                    damage @id, @target_id, @damage, 3;
                }
                else
                {
                    set_mp @sd, get_mp(@sd)-@mp;
                }
NEXT2:
                set @play, @play+1;
            }
        }
    }
    if(@play > 0)
    {
        action @id, getarg(8), 30, 0;
        sound @id, getarg(7);
        say @id, 2, @say$;
        set_hp @sd, @hp;
        set_mp @sd, get_mp(@sd)-@mp;
    }
    return;
}